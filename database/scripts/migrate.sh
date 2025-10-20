#!/bin/bash

# Database Migration Script
# This script runs database migrations in order

set -e

# Default values
DB_HOST=${DB_HOST:-localhost}
DB_PORT=${DB_PORT:-5432}
DB_NAME=${DB_NAME:-fingraph}
DB_USER=${DB_USER:-fingraph}
DB_PASSWORD=${DB_PASSWORD:-fingraph123}
MIGRATIONS_DIR=${MIGRATIONS_DIR:-$(dirname "$0")/../migrations}

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

# Function to check if migration was already applied
is_migration_applied() {
    local migration_file=$1
    local migration_name=$(basename "$migration_file" .sql)
    
    psql "host=$DB_HOST port=$DB_PORT dbname=$DB_NAME user=$DB_USER password=$DB_PASSWORD" -tAc "
        SELECT EXISTS (
            SELECT 1 FROM schema_migrations 
            WHERE migration_name = '$migration_name'
        );
    " 2>/dev/null || echo "false"
}

# Function to mark migration as applied
mark_migration_applied() {
    local migration_file=$1
    local migration_name=$(basename "$migration_file" .sql)
    
    psql "host=$DB_HOST port=$DB_PORT dbname=$DB_NAME user=$DB_USER password=$DB_PASSWORD" -c "
        INSERT INTO schema_migrations (migration_name, applied_at) 
        VALUES ('$migration_name', CURRENT_TIMESTAMP);
    " 2>/dev/null
}

# Function to create schema_migrations table if it doesn't exist
create_migrations_table() {
    print_status "Creating schema_migrations table if it doesn't exist..."
    
    psql "host=$DB_HOST port=$DB_PORT dbname=$DB_NAME user=$DB_USER password=$DB_PASSWORD" -c "
        CREATE TABLE IF NOT EXISTS schema_migrations (
            id SERIAL PRIMARY KEY,
            migration_name VARCHAR(255) NOT NULL UNIQUE,
            applied_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
        );
    " 2>/dev/null
    
    if [ $? -eq 0 ]; then
        print_status "Schema migrations table is ready"
    else
        print_error "Failed to create schema_migrations table"
        exit 1
    fi
}

# Function to run a single migration
run_migration() {
    local migration_file=$1
    
    print_status "Running migration: $(basename "$migration_file")"
    
    # Execute the migration
    psql "host=$DB_HOST port=$DB_PORT dbname=$DB_NAME user=$DB_USER password=$DB_PASSWORD" -f "$migration_file"
    
    if [ $? -eq 0 ]; then
        mark_migration_applied "$migration_file"
        print_status "Migration completed successfully: $(basename "$migration_file")"
    else
        print_error "Migration failed: $(basename "$migration_file")"
        exit 1
    fi
}

# Function to test database connection
test_connection() {
    print_status "Testing database connection..."
    
    if psql "host=$DB_HOST port=$DB_PORT dbname=$DB_NAME user=$DB_USER password=$DB_PASSWORD" -c "SELECT 1;" >/dev/null 2>&1; then
        print_status "Database connection successful"
        return 0
    else
        print_error "Cannot connect to database"
        print_error "Connection details: host=$DB_HOST port=$DB_PORT dbname=$DB_NAME user=$DB_USER"
        exit 1
    fi
}

# Main execution
main() {
    print_status "Starting database migration process..."
    
    # Test database connection
    test_connection
    
    # Create migrations table
    create_migrations_table
    
    # Get list of migration files sorted by name
    migration_files=$(find "$MIGRATIONS_DIR" -name "*.sql" -type f | sort)
    
    if [ -z "$migration_files" ]; then
        print_warning "No migration files found in $MIGRATIONS_DIR"
        exit 0
    fi
    
    # Run migrations
    migrations_run=0
    for migration_file in $migration_files; do
        if [ "$(is_migration_applied "$migration_file")" = "false" ]; then
            run_migration "$migration_file"
            migrations_run=$((migrations_run + 1))
        else
            print_status "Skipping already applied migration: $(basename "$migration_file")"
        fi
    done
    
    if [ $migrations_run -eq 0 ]; then
        print_status "No new migrations to apply"
    else
        print_status "Successfully applied $migrations_run migration(s)"
    fi
    
    print_status "Migration process completed"
}

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
        --migrations-dir)
            MIGRATIONS_DIR="$2"
            shift 2
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
            echo "  --migrations-dir DIR Migrations directory (default: ../migrations)"
            echo "  -h, --help          Show this help message"
            exit 0
            ;;
        *)
            print_error "Unknown option: $1"
            exit 1
            ;;
    esac
done

# Run main function
main