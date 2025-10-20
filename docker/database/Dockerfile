# FinGraph Database Dockerfile
# PostgreSQL 15 with custom initialization and utilities

FROM postgres:15

# Set labels
LABEL maintainer="FinGraph Team"
LABEL description="FinGraph PostgreSQL database with custom schema and utilities"

# Set environment variables
ENV POSTGRES_DB=fingraph \
    POSTGRES_USER=fingraph \
    POSTGRES_PASSWORD=fingraph123 \
    POSTGRES_INITDB_ARGS="--encoding=UTF-8 --lc-collate=C --lc-ctype=C"

# Install additional packages
RUN apt-get update && apt-get install -y \
    curl \
    jq \
    vim \
    less \
    postgresql-client-common \
    postgresql-client-15 \
    && rm -rf /var/lib/apt/lists/*

# Create directories for database files
RUN mkdir -p /docker-entrypoint-initdb.d \
    /opt/fingraph/db \
    /opt/fingraph/scripts \
    /opt/fingraph/backups \
    /opt/fingraph/logs

# Copy database schema and initialization scripts
COPY ../../database/schema.sql /docker-entrypoint-initdb.d/01-schema.sql
COPY ../../database/migrations/ /opt/fingraph/db/migrations/
COPY ../../database/seeds/ /opt/fingraph/db/seeds/
COPY ../../database/scripts/ /opt/fingraph/scripts/

# Copy custom initialization script
COPY init-db.sh /docker-entrypoint-initdb.d/02-init-db.sh

# Make scripts executable
RUN chmod +x /docker-entrypoint-initdb.d/*.sh \
    && chmod +x /opt/fingraph/scripts/*.sh

# Create custom PostgreSQL configuration
RUN cat >> /etc/postgresql/postgresql.conf << EOF

# FinGraph Custom Configuration
# Performance tuning for financial data workloads

# Memory settings
shared_buffers = 256MB
effective_cache_size = 1GB
work_mem = 4MB
maintenance_work_mem = 64MB

# Connection settings
max_connections = 200
listen_addresses = '*'

# Logging settings
log_destination = 'stderr'
logging_collector = on
log_directory = '/opt/fingraph/logs'
log_filename = 'postgresql-%Y-%m-%d_%H%M%S.log'
log_rotation_age = 1d
log_rotation_size = 100MB
log_min_duration_statement = 1000
log_checkpoints = on
log_connections = on
log_disconnections = on
log_lock_waits = on

# Performance settings
checkpoint_completion_target = 0.9
wal_buffers = 16MB
default_statistics_target = 100
random_page_cost = 1.1
effective_io_concurrency = 200

# Autovacuum settings
autovacuum = on
autovacuum_max_workers = 3
autovacuum_naptime = 1min

# Statement logging for monitoring
log_statement = 'mod'
log_min_error_statement = error

EOF

# Create pg_hba.conf for network access
RUN cat >> /etc/postgresql/pg_hba.conf << EOF

# FinGraph Network Access Rules
# Allow all connections from the Docker network
host all all 0.0.0.0/0 md5

EOF

# Create health check script
RUN cat > /opt/fingraph/scripts/health-check.sh << 'EOF'
#!/bin/bash
# Health check script for PostgreSQL

set -e

# Default values
DB_HOST=${DB_HOST:-localhost}
DB_PORT=${DB_PORT:-5432}
DB_NAME=${DB_NAME:-fingraph}
DB_USER=${DB_USER:-fingraph}
DB_PASSWORD=${DB_PASSWORD:-fingraph123}

# Check if PostgreSQL is accepting connections
if pg_isready -h "$DB_HOST" -p "$DB_PORT" -U "$DB_USER" -d "$DB_NAME"; then
    # Check if we can execute a simple query
    if PGPASSWORD="$DB_PASSWORD" psql -h "$DB_HOST" -p "$DB_PORT" -U "$DB_USER" -d "$DB_NAME" -c "SELECT 1;" >/dev/null 2>&1; then
        echo "Database is healthy"
        exit 0
    else
        echo "Database is not responding to queries"
        exit 1
    fi
else
    echo "Database is not ready"
    exit 1
fi
EOF

RUN chmod +x /opt/fingraph/scripts/health-check.sh

# Create backup script wrapper
RUN cat > /opt/fingraph/scripts/backup-wrapper.sh << 'EOF'
#!/bin/bash
# Backup script wrapper for container environment

set -e

# Set default environment variables if not provided
export DB_HOST=${DB_HOST:-localhost}
export DB_PORT=${DB_PORT:-5432}
export DB_NAME=${DB_NAME:-fingraph}
export DB_USER=${DB_USER:-fingraph}
export DB_PASSWORD=${DB_PASSWORD:-fingraph123}
export BACKUP_DIR=${BACKUP_DIR:-/opt/fingraph/backups}

# Run the backup script
exec /opt/fingraph/scripts/backup.sh "$@"
EOF

RUN chmod +x /opt/fingraph/scripts/backup-wrapper.sh

# Create migration script wrapper
RUN cat > /opt/fingraph/scripts/migrate-wrapper.sh << 'EOF'
#!/bin/bash
# Migration script wrapper for container environment

set -e

# Set default environment variables if not provided
export DB_HOST=${DB_HOST:-localhost}
export DB_PORT=${DB_PORT:-5432}
export DB_NAME=${DB_NAME:-fingraph}
export DB_USER=${DB_USER:-fingraph}
export DB_PASSWORD=${DB_PASSWORD:-fingraph123}
export MIGRATIONS_DIR=${MIGRATIONS_DIR:-/opt/fingraph/db/migrations}

# Run the migration script
exec /opt/fingraph/scripts/migrate.sh "$@"
EOF

RUN chmod +x /opt/fingraph/scripts/migrate-wrapper.sh

# Create seed script wrapper
RUN cat > /opt/fingraph/scripts/seed-wrapper.sh << 'EOF'
#!/bin/bash
# Seed script wrapper for container environment

set -e

# Set default environment variables if not provided
export DB_HOST=${DB_HOST:-localhost}
export DB_PORT=${DB_PORT:-5432}
export DB_NAME=${DB_NAME:-fingraph}
export DB_USER=${DB_USER:-fingraph}
export DB_PASSWORD=${DB_PASSWORD:-fingraph123}
export SEEDS_DIR=${SEEDS_DIR:-/opt/fingraph/db/seeds}

# Run the seed script
exec /opt/fingraph/scripts/seed.sh "$@"
EOF

RUN chmod +x /opt/fingraph/scripts/seed-wrapper.sh

# Create entrypoint script
RUN cat > /opt/fingraph/scripts/entrypoint.sh << 'EOF'
#!/bin/bash
# Custom entrypoint script for FinGraph database

set -e

echo "Starting FinGraph PostgreSQL database..."

# Run the standard PostgreSQL entrypoint
exec docker-entrypoint.sh postgres
EOF

RUN chmod +x /opt/fingraph/scripts/entrypoint.sh

# Set the entrypoint
ENTRYPOINT ["/opt/fingraph/scripts/entrypoint.sh"]

# Expose the PostgreSQL port
EXPOSE 5432

# Set volumes
VOLUME ["/var/lib/postgresql/data", "/opt/fingraph/backups", "/opt/fingraph/logs"]

# Health check
HEALTHCHECK --interval=30s --timeout=10s --start-period=5s --retries=3 \
    CMD /opt/fingraph/scripts/health-check.sh

# Default command
CMD ["postgres"]