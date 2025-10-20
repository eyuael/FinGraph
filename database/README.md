# FinGraph Database

This directory contains the complete database setup for the FinGraph financial analysis platform.

## Structure

```
database/
├── schema.sql              # Complete database schema
├── migrations/             # Database migration scripts
│   ├── 001_initial_schema.sql
│   └── 002_add_backtest_tables.sql
├── seeds/                  # Seed data files
│   └── symbols.sql
├── scripts/                # Utility scripts
│   ├── migrate.sh         # Run database migrations
│   ├── seed.sh            # Load seed data
│   └── backup.sh          # Create database backups
└── README.md              # This file
```

## Database Schema

The FinGraph database includes the following main tables:

### Core Tables

- **symbols**: Stock metadata and information
- **market_data**: OHLCV price data from AlphaVantage API
- **api_metadata**: API response metadata for tracking
- **data_ingestion_logs**: Logs for data ingestion jobs

### Backtesting Tables

- **backtest_results**: Results from trading strategy backtests
- **trades**: Individual trade records from backtests
- **performance_metrics**: Calculated performance metrics

### Views and Functions

- **latest_market_data**: View of the most recent data per symbol
- **symbol_summary**: Summary statistics for each symbol
- **calculate_symbol_stats()**: Function to calculate symbol statistics

## Quick Start

### Using Docker Setup

The easiest way to set up the database is using the provided Docker setup:

```bash
# Set up the database (builds image, starts container, runs migrations)
./docker/scripts/setup-database.sh setup

# Check database status
./docker/scripts/setup-database.sh status

# Connect to the database
./docker/scripts/setup-database.sh connect
```

### Manual Setup

If you prefer to set up the database manually:

1. **Start PostgreSQL** (using Docker or local installation)
2. **Run migrations**:
   ```bash
   cd database/scripts
   ./migrate.sh --host localhost --user fingraph --dbname fingraph
   ```
3. **Load seed data**:
   ```bash
   ./seed.sh --host localhost --user fingraph --dbname fingraph
   ```

## Environment Variables

The following environment variables can be used to configure database connections:

- `DB_HOST`: Database host (default: localhost)
- `DB_PORT`: Database port (default: 5432)
- `DB_NAME`: Database name (default: fingraph)
- `DB_USER`: Database user (default: fingraph)
- `DB_PASSWORD`: Database password (default: fingraph123)

## Scripts

### migrate.sh

Runs database migrations in order. Tracks which migrations have been applied to avoid re-running them.

```bash
# Run all pending migrations
./migrate.sh

# With custom connection parameters
./migrate.sh --host localhost --user myuser --dbname mydb
```

### seed.sh

Loads seed data into the database. Skips tables that already contain data.

```bash
# Load all seed data
./seed.sh

# With custom connection parameters
./seed.sh --host localhost --user myuser --dbname mydb
```

### backup.sh

Creates database backups with compression and automatic cleanup of old backups.

```bash
# Create a backup
./backup.sh

# Create backup with custom settings
./backup.sh --backup-dir /path/to/backups --retention 7

# Show backup statistics
./backup.sh --stats
```

## AlphaVantage Integration

The database is designed to work seamlessly with the AlphaVantage API:

### API Response Mapping

- `Meta Data` section → `api_metadata` table
- `Time Series (Daily)` section → `market_data` table
- Symbol information → `symbols` table

### Data Validation

The database includes triggers to validate OHLCV data:
- High price must be ≥ low price
- Open and close prices must be between high and low
- Volume cannot be negative

## Performance Considerations

### Indexes

The schema includes optimized indexes for common query patterns:
- Composite index on (symbol, timestamp) for time series queries
- Individual indexes on frequently queried columns
- Indexes for foreign key relationships

### Partitioning

For large datasets, consider partitioning the `market_data` table by year:

```sql
-- Example partitioning (PostgreSQL 10+)
CREATE TABLE market_data_y2023 PARTITION OF market_data
FOR VALUES FROM ('2023-01-01') TO ('2024-01-01');
```

### Data Retention

Implement data retention policies for old data:

```sql
-- Archive data older than 5 years
DELETE FROM market_data 
WHERE timestamp < CURRENT_DATE - INTERVAL '5 years';
```

## Backup and Recovery

### Automated Backups

Set up automated backups using cron:

```bash
# Daily backup at 2 AM
0 2 * * * /path/to/database/scripts/backup.sh --retention 30
```

### Recovery

Restore from backup:

```bash
# Using the setup script
./docker/scripts/setup-database.sh restore backup_file.dump

# Or manually
pg_restore -h localhost -U fingraph -d fingraph backup_file.dump
```

## Monitoring

### Health Checks

The database container includes health checks that verify:
- PostgreSQL is accepting connections
- Database responds to simple queries

### Logging

Database logs are configured to capture:
- Slow queries (>1 second)
- Connection/disconnection events
- Checkpoint activity
- Error messages

## Development vs Production

### Development Environment

- Uses `docker-compose.yml`
- Seed data is loaded automatically (`SEED_DATA=true`)
- More verbose logging
- Additional debugging tools installed

### Production Environment

- Uses `docker-compose.prod.yml`
- No automatic seed data (`SEED_DATA=false`)
- Optimized logging with rotation
- Resource limits configured
- Environment-specific configuration

## Troubleshooting

### Common Issues

1. **Connection refused**: Ensure PostgreSQL is running and ports are accessible
2. **Migration failures**: Check migration file syntax and database permissions
3. **Seed data conflicts**: Verify tables are empty before seeding

### Debug Commands

```bash
# Check database status
./docker/scripts/setup-database.sh status

# View logs
./docker/scripts/setup-database.sh logs

# Connect directly
./docker/scripts/setup-database.sh connect

# Reset database (caution: deletes all data)
./docker/scripts/setup-database.sh reset
```

## Contributing

When modifying the database schema:

1. Create a new migration file with a descriptive name
2. Use the next sequential number (e.g., `003_add_new_feature.sql`)
3. Test migrations on both empty and populated databases
4. Update this README if adding new tables or significant changes

## Security Considerations

- Use strong passwords in production
- Enable SSL connections for remote access
- Regularly update PostgreSQL to latest security patches
- Implement proper user permissions and role-based access
- Monitor for unusual query patterns or access attempts