#!/bin/bash

# Database Seeding Script
# This script runs seed data files to populate the database

set -e

# Default values
DB_HOST=${DB_HOST:-localhost}
DB_PORT=${DB_PORT:-5432}
DB_NAME=${DB_NAME:-fingraph}
DB_USER=${DB_USER:-fingraph}
DB_PASSWORD=${DB_PASSWORD:-fingraph123}
SEEDS_DIR=${SEEDS_DIR:-$(dirname "$0")/../seeds}

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
    
    if psql "host=$DB_HOST port=$DB_PORT dbname=$DB_NAME user=$DB_USER password=$DB_PASSWORD" -c "SELECT 1;" >/dev/null 2>&1; then
        print_status "Database connection successful"
        return 0
    else
        print_error "Cannot connect to database"
        print_error "Connection details: host=$DB_HOST port=$DB_PORT dbname=$DB_NAME user=$DB_USER"
        exit 1
    fi
}

# Function to run a single seed file
run_seed() {
    local seed_file=$1
    
    print_status "Running seed: $(basename "$seed_file")"
    
    # Execute the seed
    psql "host=$DB_HOST port=$DB_PORT dbname=$DB_NAME user=$DB_USER password=$DB_PASSWORD" -f "$seed_file"
    
    if [ $? -eq 0 ]; then
        print_status "Seed completed successfully: $(basename "$seed_file")"
    else
        print_error "Seed failed: $(basename "$seed_file")"
        exit 1
    fi
}

# Function to check if table has data
has_data() {
    local table_name=$1
    local count=$(psql "host=$DB_HOST port=$DB_PORT dbname=$DB_NAME user=$DB_USER password=$DB_PASSWORD" -tAc "SELECT COUNT(*) FROM $table_name;" 2>/dev/null || echo "0")
    [ "$count" -gt 0 ]
}

# Main execution
main() {
    print_status "Starting database seeding process..."
    
    # Test database connection
    test_connection
    
    # Get list of seed files sorted by name
    seed_files=$(find "$SEEDS_DIR" -name "*.sql" -type f | sort)
    
    if [ -z "$seed_files" ]; then
        print_warning "No seed files found in $SEEDS_DIR"
        exit 0
    fi
    
    # Run seeds
    seeds_run=0
    for seed_file in $seed_files; do
        seed_name=$(basename "$seed_file" .sql)
        
        # Check if we should skip this seed based on existing data
        case "$seed_name" in
            "symbols")
                if has_data "symbols"; then
                    print_status "Skipping symbols seed - table already has data"
                    continue
                fi
                ;;
        esac
        
        run_seed "$seed_file"
        seeds_run=$((seeds_run + 1))
    done
    
    if [ $seeds_run -eq 0 ]; then
        print_status "No new seeds to apply"
    else
        print_status "Successfully applied $seeds_run seed(s)"
    fi
    
    print_status "Seeding process completed"
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
        --seeds-dir)
            SEEDS_DIR="$2"
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
            echo "  --seeds-dir DIR     Seeds directory (default: ../seeds)"
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