#!/bin/bash

# FinGraph Database Setup Script
# This script sets up and deploys the FinGraph database

set -e

# Default values
ENVIRONMENT=${ENVIRONMENT:-development}
COMPOSE_FILE=${COMPOSE_FILE:-docker-compose.yml}
DB_HOST=${DB_HOST:-localhost}
DB_PORT=${DB_PORT:-5432}
DB_NAME=${DB_NAME:-fingraph}
DB_USER=${DB_USER:-fingraph}
DB_PASSWORD=${DB_PASSWORD:-fingraph123}
SEED_DATA=${SEED_DATA:-true}
BUILD_IMAGE=${BUILD_IMAGE:-true}

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Function to print colored output
print_status() {
    echo -e "${GREEN}[SETUP]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[SETUP]${NC} $1"
}

print_error() {
    echo -e "${RED}[SETUP]${NC} $1"
}

print_info() {
    echo -e "${BLUE}[SETUP]${NC} $1"
}

# Function to check if Docker is running
check_docker() {
    if ! docker info >/dev/null 2>&1; then
        print_error "Docker is not running. Please start Docker first."
        exit 1
    fi
    print_status "Docker is running"
}

# Function to check if docker-compose is available
check_docker_compose() {
    if ! command -v docker-compose >/dev/null 2>&1 && ! docker compose version >/dev/null 2>&1; then
        print_error "docker-compose is not installed or not in PATH"
        exit 1
    fi
    
    # Use docker compose if available, otherwise docker-compose
    if docker compose version >/dev/null 2>&1; then
        DOCKER_COMPOSE="docker compose"
    else
        DOCKER_COMPOSE="docker-compose"
    fi
    
    print_status "Using: $DOCKER_COMPOSE"
}

# Function to build database image
build_database_image() {
    if [ "$BUILD_IMAGE" = "true" ]; then
        print_status "Building FinGraph database image..."
        $DOCKER_COMPOSE -f "$COMPOSE_FILE" build postgres
        if [ $? -eq 0 ]; then
            print_status "Database image built successfully"
        else
            print_error "Failed to build database image"
            exit 1
        fi
    else
        print_status "Skipping database image build (BUILD_IMAGE=false)"
    fi
}

# Function to start database container
start_database() {
    print_status "Starting FinGraph database container..."
    
    # Set environment variables for seeding
    export SEED_DATA="$SEED_DATA"
    
    $DOCKER_COMPOSE -f "$COMPOSE_FILE" up -d postgres
    
    if [ $? -eq 0 ]; then
        print_status "Database container started successfully"
    else
        print_error "Failed to start database container"
        exit 1
    fi
}

# Function to wait for database to be ready
wait_for_database() {
    print_status "Waiting for database to be ready..."
    
    local max_attempts=30
    local attempt=1
    
    while [ $attempt -le $max_attempts ]; do
        if $DOCKER_COMPOSE -f "$COMPOSE_FILE" exec -T postgres pg_isready -h localhost -p 5432 -U "$DB_USER" -d "$DB_NAME" >/dev/null 2>&1; then
            print_status "Database is ready!"
            return 0
        fi
        
        print_info "Attempt $attempt/$max_attempts: Database not ready yet..."
        sleep 2
        attempt=$((attempt + 1))
    done
    
    print_error "Database failed to become ready after $max_attempts attempts"
    exit 1
}

# Function to test database connection
test_database_connection() {
    print_status "Testing database connection..."
    
    if $DOCKER_COMPOSE -f "$COMPOSE_FILE" exec -T postgres psql -h localhost -p 5432 -U "$DB_USER" -d "$DB_NAME" -c "SELECT 1;" >/dev/null 2>&1; then
        print_status "Database connection successful"
    else
        print_error "Database connection failed"
        exit 1
    fi
}

# Function to show database status
show_database_status() {
    print_status "Database Status:"
    $DOCKER_COMPOSE -f "$COMPOSE_FILE" ps postgres
    
    print_status "Database Information:"
    $DOCKER_COMPOSE -f "$COMPOSE_FILE" exec -T postgres psql -h localhost -p 5432 -U "$DB_USER" -d "$DB_NAME" -c "
        SELECT 
            'Database Size: ' || pg_size_pretty(pg_database_size('$DB_NAME')) as info
        UNION ALL
        SELECT 
            'Connections: ' || count(*)::text 
        FROM pg_stat_activity 
        WHERE datname = '$DB_NAME'
        UNION ALL
        SELECT 
            'Tables: ' || count(*)::text 
        FROM information_schema.tables 
        WHERE table_schema = 'public'
        UNION ALL
        SELECT 
            'Migrations Applied: ' || count(*)::text 
        FROM schema_migrations;
    " 2>/dev/null || print_warning "Could not retrieve database information"
}

# Function to stop database
stop_database() {
    print_status "Stopping FinGraph database container..."
    $DOCKER_COMPOSE -f "$COMPOSE_FILE" stop postgres
    if [ $? -eq 0 ]; then
        print_status "Database container stopped"
    else
        print_error "Failed to stop database container"
        exit 1
    fi
}

# Function to remove database container and volumes
remove_database() {
    print_warning "This will remove the database container and all data!"
    read -p "Are you sure you want to continue? (y/N): " -n 1 -r
    echo
    
    if [[ $REPLY =~ ^[Yy]$ ]]; then
        print_status "Removing FinGraph database container and volumes..."
        $DOCKER_COMPOSE -f "$COMPOSE_FILE" down -v postgres
        if [ $? -eq 0 ]; then
            print_status "Database container and volumes removed"
        else
            print_error "Failed to remove database container and volumes"
            exit 1
        fi
    else
        print_status "Database removal cancelled"
    fi
}

# Function to reset database
reset_database() {
    print_warning "This will reset the database (remove all data and recreate)!"
    read -p "Are you sure you want to continue? (y/N): " -n 1 -r
    echo
    
    if [[ $REPLY =~ ^[Yy]$ ]]; then
        print_status "Resetting FinGraph database..."
        
        # Stop and remove container with volumes
        $DOCKER_COMPOSE -f "$COMPOSE_FILE" down -v postgres
        
        # Build and start fresh
        build_database_image
        start_database
        wait_for_database
        test_database_connection
        show_database_status
        
        print_status "Database reset completed"
    else
        print_status "Database reset cancelled"
    fi
}

# Function to backup database
backup_database() {
    print_status "Creating database backup..."
    
    local backup_dir="./backups"
    mkdir -p "$backup_dir"
    
    local timestamp=$(date +"%Y%m%d_%H%M%S")
    local backup_file="$backup_dir/${DB_NAME}_backup_${timestamp}.dump"
    
    $DOCKER_COMPOSE -f "$COMPOSE_FILE" exec -T postgres pg_dump \
        -h localhost -p 5432 -U "$DB_USER" -d "$DB_NAME" \
        -F custom -Z 9 -f "/tmp/backup.dump"
    
    $DOCKER_COMPOSE -f "$COMPOSE_FILE" exec -T postgres cat "/tmp/backup.dump" > "$backup_file"
    
    if [ $? -eq 0 ]; then
        local backup_size=$(du -h "$backup_file" | cut -f1)
        print_status "Backup created successfully: $backup_file ($backup_size)"
    else
        print_error "Backup failed"
        exit 1
    fi
}

# Function to restore database
restore_database() {
    local backup_file=$1
    
    if [ -z "$backup_file" ]; then
        print_error "Please specify a backup file to restore"
        exit 1
    fi
    
    if [ ! -f "$backup_file" ]; then
        print_error "Backup file not found: $backup_file"
        exit 1
    fi
    
    print_warning "This will replace the current database with the backup!"
    read -p "Are you sure you want to continue? (y/N): " -n 1 -r
    echo
    
    if [[ $REPLY =~ ^[Yy]$ ]]; then
        print_status "Restoring database from: $backup_file"
        
        # Copy backup file to container
        $DOCKER_COMPOSE -f "$COMPOSE_FILE" exec -T postgres sh -c "cat > /tmp/restore.dump" < "$backup_file"
        
        # Restore database
        $DOCKER_COMPOSE -f "$COMPOSE_FILE" exec -T postgres pg_restore \
            -h localhost -p 5432 -U "$DB_USER" -d "$DB_NAME" \
            --clean --if-exists --verbose "/tmp/restore.dump"
        
        if [ $? -eq 0 ]; then
            print_status "Database restored successfully"
        else
            print_error "Database restore failed"
            exit 1
        fi
    else
        print_status "Database restore cancelled"
    fi
}

# Function to show logs
show_logs() {
    print_status "Showing database logs..."
    $DOCKER_COMPOSE -f "$COMPOSE_FILE" logs -f postgres
}

# Function to connect to database
connect_database() {
    print_status "Connecting to FinGraph database..."
    $DOCKER_COMPOSE -f "$COMPOSE_FILE" exec postgres psql -h localhost -p 5432 -U "$DB_USER" -d "$DB_NAME"
}

# Function to show help
show_help() {
    echo "FinGraph Database Setup Script"
    echo ""
    echo "Usage: $0 [COMMAND] [OPTIONS]"
    echo ""
    echo "Commands:"
    echo "  setup       Set up and start the database (default)"
    echo "  start       Start the database container"
    echo "  stop        Stop the database container"
    echo "  restart     Restart the database container"
    echo "  status      Show database status"
    echo "  reset       Reset the database (remove all data)"
    echo "  remove      Remove database container and volumes"
    echo "  backup      Create a database backup"
    echo "  restore     Restore database from backup"
    echo "  logs        Show database logs"
    echo "  connect     Connect to database using psql"
    echo "  help        Show this help message"
    echo ""
    echo "Environment Variables:"
    echo "  ENVIRONMENT     Environment (development|production) [default: development]"
    echo "  COMPOSE_FILE    Docker compose file [default: docker-compose.yml]"
    echo "  DB_HOST         Database host [default: localhost]"
    echo "  DB_PORT         Database port [default: 5432]"
    echo "  DB_NAME         Database name [default: fingraph]"
    echo "  DB_USER         Database user [default: fingraph]"
    echo "  DB_PASSWORD     Database password [default: fingraph123]"
    echo "  SEED_DATA       Load seed data [default: true]"
    echo "  BUILD_IMAGE     Build database image [default: true]"
    echo ""
    echo "Examples:"
    echo "  $0 setup                    # Set up database with defaults"
    echo "  $0 setup ENVIRONMENT=prod   # Set up production database"
    echo "  $0 backup                   # Create backup"
    echo "  $0 restore backup.dump       # Restore from backup"
    echo "  $0 connect                  # Connect to database"
}

# Main execution
main() {
    local command=${1:-setup}
    
    # Check dependencies
    check_docker
    check_docker_compose
    
    case "$command" in
        "setup")
            print_status "Setting up FinGraph database..."
            build_database_image
            start_database
            wait_for_database
            test_database_connection
            show_database_status
            print_status "FinGraph database setup completed successfully!"
            ;;
        "start")
            start_database
            wait_for_database
            test_database_connection
            ;;
        "stop")
            stop_database
            ;;
        "restart")
            stop_database
            start_database
            wait_for_database
            test_database_connection
            ;;
        "status")
            show_database_status
            ;;
        "reset")
            reset_database
            ;;
        "remove")
            remove_database
            ;;
        "backup")
            backup_database
            ;;
        "restore")
            restore_database "$2"
            ;;
        "logs")
            show_logs
            ;;
        "connect")
            connect_database
            ;;
        "help"|"-h"|"--help")
            show_help
            ;;
        *)
            print_error "Unknown command: $command"
            echo ""
            show_help
            exit 1
            ;;
    esac
}

# Run main function with all arguments
main "$@"