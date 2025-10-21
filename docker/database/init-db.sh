#!/bin/bash

# FinGraph Database Initialization Script
# This script runs after PostgreSQL is initialized to set up the FinGraph database

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Function to print colored output
print_status() {
    echo -e "${GREEN}[INIT]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[INIT]${NC} $1"
}

print_error() {
    echo -e "${RED}[INIT]${NC} $1"
}

print_status "Starting FinGraph database initialization..."

# Check if we're running as the postgres user
if [ "$POSTGRES_USER" != "postgres" ]; then
    print_status "Switching to postgres user for initialization..."
    exec gosu postgres "$0" "$@"
fi

# Wait for PostgreSQL to be ready
print_status "Waiting for PostgreSQL to be ready..."
while ! pg_isready -q; do
    sleep 1
done

print_status "PostgreSQL is ready. Initializing FinGraph database..."

# Create the database if it doesn't exist
if ! psql -lqt | cut -d \| -f 1 | grep -qw "$POSTGRES_DB"; then
    print_status "Creating database: $POSTGRES_DB"
    createdb "$POSTGRES_DB"
else
    print_status "Database $POSTGRES_DB already exists"
fi

# Connect to the database and run initialization
print_status "Setting up database extensions and configuration..."

psql -v ON_ERROR_STOP=1 --username "$POSTGRES_USER" --dbname "$POSTGRES_DB" <<-EOSQL
    -- Enable required extensions
    CREATE EXTENSION IF NOT EXISTS "uuid-ossp";
    CREATE EXTENSION IF NOT EXISTS "pg_stat_statements";
    
    -- Create schema_migrations table for tracking migrations
    CREATE TABLE IF NOT EXISTS schema_migrations (
        id SERIAL PRIMARY KEY,
        migration_name VARCHAR(255) NOT NULL UNIQUE,
        applied_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
    );
    
    -- Set up permissions
    GRANT ALL PRIVILEGES ON ALL TABLES IN SCHEMA public TO $POSTGRES_USER;
    GRANT ALL PRIVILEGES ON ALL SEQUENCES IN SCHEMA public TO $POSTGRES_USER;
    
    -- Create initial indexes for performance
    CREATE INDEX IF NOT EXISTS idx_schema_migrations_name ON schema_migrations(migration_name);
    
    -- Log initialization
    INSERT INTO schema_migrations (migration_name) VALUES ('initialization') 
    ON CONFLICT (migration_name) DO NOTHING;
EOSQL

if [ $? -eq 0 ]; then
    print_status "Database initialization completed successfully"
else
    print_error "Database initialization failed"
    exit 1
fi

# Run migrations if they exist
if [ -d "/opt/fingraph/db/migrations" ] && [ "$(ls -A /opt/fingraph/db/migrations)" ]; then
    print_status "Running database migrations..."
    
    # Find and run migration files in order
    for migration_file in /opt/fingraph/db/migrations/*.sql; do
        if [ -f "$migration_file" ]; then
            migration_name=$(basename "$migration_file" .sql)
            
            # Check if migration was already applied
            migration_applied=$(psql -tAc "SELECT EXISTS (SELECT 1 FROM schema_migrations WHERE migration_name = '$migration_name');" "$POSTGRES_DB")
            
            if [ "$migration_applied" = "t" ]; then
                print_status "Skipping already applied migration: $migration_name"
            else
                print_status "Running migration: $migration_name"
                psql -v ON_ERROR_STOP=1 --username "$POSTGRES_USER" --dbname "$POSTGRES_DB" -f "$migration_file"
                
                if [ $? -eq 0 ]; then
                    # Mark migration as applied
                    psql -v ON_ERROR_STOP=1 --username "$POSTGRES_USER" --dbname "$POSTGRES_DB" -c "INSERT INTO schema_migrations (migration_name) VALUES ('$migration_name');"
                    print_status "Migration completed: $migration_name"
                else
                    print_error "Migration failed: $migration_name"
                    exit 1
                fi
            fi
        fi
    done
    
    print_status "All migrations completed"
else
    print_warning "No migration files found"
fi

# Run seed data if it exists and if SEED_DATA environment variable is set to true
if [ "$SEED_DATA" = "true" ] && [ -d "/opt/fingraph/db/seeds" ] && [ "$(ls -A /opt/fingraph/db/seeds)" ]; then
    print_status "Running seed data..."
    
    # Find and run seed files in order
    for seed_file in /opt/fingraph/db/seeds/*.sql; do
        if [ -f "$seed_file" ]; then
            seed_name=$(basename "$seed_file" .sql)
            print_status "Running seed: $seed_name"
            psql -v ON_ERROR_STOP=1 --username "$POSTGRES_USER" --dbname "$POSTGRES_DB" -f "$seed_file"
            
            if [ $? -eq 0 ]; then
                print_status "Seed completed: $seed_name"
            else
                print_error "Seed failed: $seed_name"
                exit 1
            fi
        fi
    done
    
    print_status "All seed data completed"
else
    if [ "$SEED_DATA" != "true" ]; then
        print_status "Seed data skipped (SEED_DATA not set to true)"
    else
        print_warning "No seed files found"
    fi
fi

# Create backup directory
mkdir -p /opt/fingraph/backups

# Create log directory
mkdir -p /opt/fingraph/logs

# Set proper permissions
chown -R postgres:postgres /opt/fingraph

print_status "FinGraph database initialization completed successfully!"
print_status "Database: $POSTGRES_DB"
print_status "User: $POSTGRES_USER"
print_status "Port: 5432"

# Show database statistics
print_status "Database statistics:"
psql -v ON_ERROR_STOP=1 --username "$POSTGRES_USER" --dbname "$POSTGRES_DB" -c "
    SELECT 
        schemaname,
        tablename,
        n_tup_ins as inserts,
        n_tup_upd as updates,
        n_tup_del as deletes
    FROM pg_stat_user_tables 
    ORDER BY schemaname, tablename;
" 2>/dev/null || print_warning "Could not retrieve database statistics"

print_status "Initialization complete. Database is ready for use."