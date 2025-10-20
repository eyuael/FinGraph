#!/bin/bash

# Database Backup Script
# This script creates backups of the FinGraph database

set -e

# Default values
DB_HOST=${DB_HOST:-localhost}
DB_PORT=${DB_PORT:-5432}
DB_NAME=${DB_NAME:-fingraph}
DB_USER=${DB_USER:-fingraph}
DB_PASSWORD=${DB_PASSWORD:-fingraph123}
BACKUP_DIR=${BACKUP_DIR:-./backups}
BACKUP_FORMAT=${BACKUP_FORMAT:-custom} # custom, directory, plain, tar
COMPRESS=${COMPRESS:-9} # Compression level 0-9
RETENTION_DAYS=${RETENTION_DAYS:-30}

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Function to print colored output
print_status() {
    echo -e "${GREEN}[INFO]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Function to test database connection
test_connection() {
    print_status "Testing database connection..."
    
    if PGPASSWORD="$DB_PASSWORD" pg_isready -h "$DB_HOST" -p "$DB_PORT" -U "$DB_USER" -d "$DB_NAME" >/dev/null 2>&1; then
        print_status "Database connection successful"
        return 0
    else
        print_error "Cannot connect to database"
        print_error "Connection details: host=$DB_HOST port=$DB_PORT dbname=$DB_NAME user=$DB_USER"
        exit 1
    fi
}

# Function to create backup directory
create_backup_dir() {
    if [ ! -d "$BACKUP_DIR" ]; then
        mkdir -p "$BACKUP_DIR"
        print_status "Created backup directory: $BACKUP_DIR"
    fi
}

# Function to create database backup
create_backup() {
    local timestamp=$(date +"%Y%m%d_%H%M%S")
    local backup_file="$BACKUP_DIR/${DB_NAME}_backup_${timestamp}.dump"
    
    print_status "Creating database backup..."
    print_status "Backup file: $backup_file"
    
    # Create the backup
    PGPASSWORD="$DB_PASSWORD" pg_dump \
        -h "$DB_HOST" \
        -p "$DB_PORT" \
        -U "$DB_USER" \
        -d "$DB_NAME" \
        -F "$BACKUP_FORMAT" \
        -Z "$COMPRESS" \
        -v \
        -f "$backup_file"
    
    if [ $? -eq 0 ]; then
        local backup_size=$(du -h "$backup_file" | cut -f1)
        print_status "Backup created successfully"
        print_status "Backup size: $backup_size"
        print_status "Backup file: $backup_file"
        
        # Create a symlink to the latest backup
        local latest_link="$BACKUP_DIR/${DB_NAME}_latest.dump"
        ln -sf "$(basename "$backup_file")" "$latest_link"
        
        return 0
    else
        print_error "Backup failed"
        return 1
    fi
}

# Function to create schema-only backup
create_schema_backup() {
    local timestamp=$(date +"%Y%m%d_%H%M%S")
    local schema_file="$BACKUP_DIR/${DB_NAME}_schema_${timestamp}.sql"
    
    print_status "Creating schema-only backup..."
    print_status "Schema file: $schema_file"
    
    # Create schema backup
    PGPASSWORD="$DB_PASSWORD" pg_dump \
        -h "$DB_HOST" \
        -p "$DB_PORT" \
        -U "$DB_USER" \
        -d "$DB_NAME" \
        --schema-only \
        --no-owner \
        --no-privileges \
        -f "$schema_file"
    
    if [ $? -eq 0 ]; then
        print_status "Schema backup created successfully: $schema_file"
        return 0
    else
        print_error "Schema backup failed"
        return 1
    fi
}

# Function to clean old backups
cleanup_old_backups() {
    print_status "Cleaning up old backups (older than $RETENTION_DAYS days)..."
    
    local deleted_count=0
    while IFS= read -r -d '' file; do
        print_status "Deleting old backup: $(basename "$file")"
        rm "$file"
        deleted_count=$((deleted_count + 1))
    done < <(find "$BACKUP_DIR" -name "${DB_NAME}_backup_*.dump" -type f -mtime +$RETENTION_DAYS -print0)
    
    if [ $deleted_count -gt 0 ]; then
        print_status "Deleted $deleted_count old backup(s)"
    else
        print_status "No old backups to delete"
    fi
}

# Function to verify backup
verify_backup() {
    local backup_file=$1
    
    print_status "Verifying backup integrity..."
    
    # For custom format, we can use pg_restore to list contents
    if [ "$BACKUP_FORMAT" = "custom" ]; then
        if PGPASSWORD="$DB_PASSWORD" pg_restore -l "$backup_file" >/dev/null 2>&1; then
            print_status "Backup verification successful"
            return 0
        else
            print_error "Backup verification failed"
            return 1
        fi
    else
        # For other formats, just check if file exists and is not empty
        if [ -s "$backup_file" ]; then
            print_status "Backup verification successful"
            return 0
        else
            print_error "Backup verification failed - file is empty or missing"
            return 1
        fi
    fi
}

# Function to show backup statistics
show_backup_stats() {
    print_status "Backup directory statistics:"
    
    local total_backups=$(find "$BACKUP_DIR" -name "${DB_NAME}_backup_*.dump" -type f | wc -l)
    local total_size=$(du -sh "$BACKUP_DIR" 2>/dev/null | cut -f1 || echo "Unknown")
    
    echo "  Total backups: $total_backups"
    echo "  Total size: $total_size"
    echo "  Retention period: $RETENTION_DAYS days"
    
    if [ -f "$BACKUP_DIR/${DB_NAME}_latest.dump" ]; then
        local latest_size=$(du -h "$BACKUP_DIR/${DB_NAME}_latest.dump" | cut -f1)
        local latest_date=$(stat -f "%Sm" -t "%Y-%m-%d %H:%M:%S" "$BACKUP_DIR/${DB_NAME}_latest.dump" 2>/dev/null || stat -c "%y" "$BACKUP_DIR/${DB_NAME}_latest.dump" 2>/dev/null | cut -d' ' -f1-2 || echo "Unknown")
        echo "  Latest backup: $latest_size ($latest_date)"
    fi
}

# Main execution
main() {
    local create_schema=false
    local cleanup_only=false
    local stats_only=false
    
    # Parse command line arguments
    while [[ $# -gt 0 ]]; do
        case $1 in
            --host)
                DB_HOST="$2"
                shift 2
                ;;
            --port)
                DB_PORT="$2"
                shift 2
                ;;
            --dbname)
                DB_NAME="$2"
                shift 2
                ;;
            --user)
                DB_USER="$2"
                shift 2
                ;;
            --password)
                DB_PASSWORD="$2"
                shift 2
                ;;
            --backup-dir)
                BACKUP_DIR="$2"
                shift 2
                ;;
            --format)
                BACKUP_FORMAT="$2"
                shift 2
                ;;
            --compress)
                COMPRESS="$2"
                shift 2
                ;;
            --retention)
                RETENTION_DAYS="$2"
                shift 2
                ;;
            --schema)
                create_schema=true
                shift
                ;;
            --cleanup)
                cleanup_only=true
                shift
                ;;
            --stats)
                stats_only=true
                shift
                ;;
            -h|--help)
                echo "Usage: $0 [OPTIONS]"
                echo ""
                echo "Options:"
                echo "  --host HOST         Database host (default: localhost)"
                echo "  --port PORT         Database port (default: 5432)"
                echo "  --dbname DBNAME     Database name (default: fingraph)"
                echo "  --user USER         Database user (default: fingraph)"
                echo "  --password PASSWORD Database password (default: fingraph123)"
                echo "  --backup-dir DIR    Backup directory (default: ./backups)"
                echo "  --format FORMAT     Backup format: custom, directory, plain, tar (default: custom)"
                echo "  --compress LEVEL    Compression level 0-9 (default: 9)"
                echo "  --retention DAYS    Retention period in days (default: 30)"
                echo "  --schema            Also create schema-only backup"
                echo "  --cleanup           Only cleanup old backups"
                echo "  --stats             Show backup statistics only"
                echo "  -h, --help          Show this help message"
                exit 0
                ;;
            *)
                print_error "Unknown option: $1"
                exit 1
                ;;
        esac
    done
    
    # Show statistics if requested
    if [ "$stats_only" = true ]; then
        create_backup_dir
        show_backup_stats
        exit 0
    fi
    
    # Cleanup only if requested
    if [ "$cleanup_only" = true ]; then
        create_backup_dir
        cleanup_old_backups
        exit 0
    fi
    
    print_status "Starting database backup process..."
    
    # Test database connection
    test_connection
    
    # Create backup directory
    create_backup_dir
    
    # Create main backup
    local backup_file
    backup_file=$(create_backup)
    
    if [ $? -eq 0 ]; then
        # Verify backup
        verify_backup "$backup_file"
        
        # Create schema backup if requested
        if [ "$create_schema" = true ]; then
            create_schema_backup
        fi
        
        # Clean old backups
        cleanup_old_backups
        
        # Show statistics
        show_backup_stats
        
        print_status "Backup process completed successfully"
    else
        print_error "Backup process failed"
        exit 1
    fi
}

# Run main function with all arguments
main "$@"