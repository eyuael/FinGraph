-- FinGraph Database Schema
-- PostgreSQL 15

-- Enable required extensions
CREATE EXTENSION IF NOT EXISTS "uuid-ossp";
CREATE EXTENSION IF NOT EXISTS "pg_stat_statements";

-- Create custom types
CREATE TYPE ingestion_status AS ENUM ('SUCCESS', 'FAILED', 'PARTIAL');
CREATE TYPE data_source AS ENUM ('ALPHAVANTAGE', 'MANUAL', 'IMPORT');

-- Symbols table for stock metadata
CREATE TABLE symbols (
    symbol VARCHAR(10) PRIMARY KEY,
    name VARCHAR(255) NOT NULL,
    exchange VARCHAR(50),
    sector VARCHAR(100),
    industry VARCHAR(100),
    market_cap BIGINT,
    description TEXT,
    is_active BOOLEAN DEFAULT TRUE,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- Market data table for OHLCV data
CREATE TABLE market_data (
    id SERIAL PRIMARY KEY,
    symbol VARCHAR(10) NOT NULL,
    timestamp DATE NOT NULL,
    open_price DECIMAL(12,4) NOT NULL,
    high_price DECIMAL(12,4) NOT NULL,
    low_price DECIMAL(12,4) NOT NULL,
    close_price DECIMAL(12,4) NOT NULL,
    volume BIGINT NOT NULL,
    adjusted_close DECIMAL(12,4),
    data_source data_source DEFAULT 'ALPHAVANTAGE',
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (symbol) REFERENCES symbols(symbol) ON DELETE CASCADE,
    UNIQUE(symbol, timestamp)
);

-- API metadata table for tracking API responses
CREATE TABLE api_metadata (
    id SERIAL PRIMARY KEY,
    symbol VARCHAR(10) NOT NULL,
    last_refreshed TIMESTAMP NOT NULL,
    output_size VARCHAR(20),
    time_zone VARCHAR(50),
    information TEXT,
    data_source data_source DEFAULT 'ALPHAVANTAGE',
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (symbol) REFERENCES symbols(symbol) ON DELETE CASCADE
);

-- Data ingestion logs for tracking ingestion jobs
CREATE TABLE data_ingestion_logs (
    id SERIAL PRIMARY KEY,
    symbol VARCHAR(10) NOT NULL,
    records_processed INTEGER NOT NULL,
    records_inserted INTEGER NOT NULL,
    records_updated INTEGER DEFAULT 0,
    status ingestion_status NOT NULL,
    error_message TEXT,
    processing_time_ms INTEGER,
    data_source data_source DEFAULT 'ALPHAVANTAGE',
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (symbol) REFERENCES symbols(symbol) ON DELETE CASCADE
);

-- Backtest results table
CREATE TABLE backtest_results (
    id UUID PRIMARY KEY DEFAULT uuid_generate_v4(),
    symbol VARCHAR(10) NOT NULL,
    strategy_name VARCHAR(100) NOT NULL,
    start_date DATE NOT NULL,
    end_date DATE NOT NULL,
    initial_capital DECIMAL(15,2) NOT NULL,
    final_capital DECIMAL(15,2) NOT NULL,
    total_return DECIMAL(8,4) NOT NULL,
    max_drawdown DECIMAL(8,4) NOT NULL,
    sharpe_ratio DECIMAL(8,4),
    win_rate DECIMAL(5,4),
    total_trades INTEGER NOT NULL,
    winning_trades INTEGER DEFAULT 0,
    losing_trades INTEGER DEFAULT 0,
    strategy_params JSONB,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (symbol) REFERENCES symbols(symbol) ON DELETE CASCADE
);

-- Trades table for individual trade records
CREATE TABLE trades (
    id UUID PRIMARY KEY DEFAULT uuid_generate_v4(),
    backtest_id UUID NOT NULL,
    symbol VARCHAR(10) NOT NULL,
    entry_date DATE NOT NULL,
    exit_date DATE,
    entry_price DECIMAL(12,4) NOT NULL,
    exit_price DECIMAL(12,4),
    quantity INTEGER NOT NULL,
    trade_type VARCHAR(10) NOT NULL, -- 'LONG' or 'SHORT'
    profit_loss DECIMAL(12,2),
    profit_loss_percent DECIMAL(8,4),
    status VARCHAR(20) NOT NULL, -- 'OPEN', 'CLOSED', 'CANCELLED'
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (backtest_id) REFERENCES backtest_results(id) ON DELETE CASCADE,
    FOREIGN KEY (symbol) REFERENCES symbols(symbol) ON DELETE CASCADE
);

-- Performance metrics table for storing calculated metrics
CREATE TABLE performance_metrics (
    id UUID PRIMARY KEY DEFAULT uuid_generate_v4(),
    backtest_id UUID NOT NULL,
    metric_name VARCHAR(50) NOT NULL,
    metric_value DECIMAL(15,6) NOT NULL,
    metric_period VARCHAR(20), -- 'daily', 'weekly', 'monthly', 'yearly'
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (backtest_id) REFERENCES backtest_results(id) ON DELETE CASCADE
);

-- Create indexes for performance
CREATE INDEX idx_market_data_symbol_timestamp ON market_data(symbol, timestamp DESC);
CREATE INDEX idx_market_data_timestamp ON market_data(timestamp DESC);
CREATE INDEX idx_market_data_symbol ON market_data(symbol);
CREATE INDEX idx_symbols_active ON symbols(is_active);
CREATE INDEX idx_api_metadata_symbol ON api_metadata(symbol);
CREATE INDEX idx_api_metadata_last_refreshed ON api_metadata(last_refreshed DESC);
CREATE INDEX idx_data_ingestion_logs_symbol ON data_ingestion_logs(symbol);
CREATE INDEX idx_data_ingestion_logs_created_at ON data_ingestion_logs(created_at DESC);
CREATE INDEX idx_backtest_results_symbol ON backtest_results(symbol);
CREATE INDEX idx_backtest_results_strategy ON backtest_results(strategy_name);
CREATE INDEX idx_backtest_results_created_at ON backtest_results(created_at DESC);
CREATE INDEX idx_trades_backtest_id ON trades(backtest_id);
CREATE INDEX idx_trades_symbol ON trades(symbol);
CREATE INDEX idx_trades_entry_date ON trades(entry_date DESC);
CREATE INDEX idx_performance_metrics_backtest_id ON performance_metrics(backtest_id);

-- Create triggers for updated_at timestamps
CREATE OR REPLACE FUNCTION update_updated_at_column()
RETURNS TRIGGER AS $$
BEGIN
    NEW.updated_at = CURRENT_TIMESTAMP;
    RETURN NEW;
END;
$$ language 'plpgsql';

CREATE TRIGGER update_symbols_updated_at BEFORE UPDATE ON symbols
    FOR EACH ROW EXECUTE FUNCTION update_updated_at_column();

CREATE TRIGGER update_market_data_updated_at BEFORE UPDATE ON market_data
    FOR EACH ROW EXECUTE FUNCTION update_updated_at_column();

-- Create views for common queries
CREATE VIEW latest_market_data AS
SELECT DISTINCT ON (symbol) 
    symbol,
    timestamp,
    close_price,
    volume,
    created_at
FROM market_data 
ORDER BY symbol, timestamp DESC;

CREATE VIEW symbol_summary AS
SELECT 
    s.symbol,
    s.name,
    s.exchange,
    s.sector,
    s.industry,
    COUNT(md.id) as data_points,
    MIN(md.timestamp) as earliest_date,
    MAX(md.timestamp) as latest_date,
    MAX(md.timestamp) = CURRENT_DATE as has_today_data
FROM symbols s
LEFT JOIN market_data md ON s.symbol = md.symbol
WHERE s.is_active = true
GROUP BY s.symbol, s.name, s.exchange, s.sector, s.industry;

-- Create functions for data validation
CREATE OR REPLACE FUNCTION validate_ohlcv_data()
RETURNS TRIGGER AS $$
BEGIN
    -- Validate OHLC relationships
    IF NEW.high_price < NEW.low_price THEN
        RAISE EXCEPTION 'High price cannot be less than low price';
    END IF;
    
    IF NEW.open_price > NEW.high_price OR NEW.open_price < NEW.low_price THEN
        RAISE EXCEPTION 'Open price must be between high and low prices';
    END IF;
    
    IF NEW.close_price > NEW.high_price OR NEW.close_price < NEW.low_price THEN
        RAISE EXCEPTION 'Close price must be between high and low prices';
    END IF;
    
    IF NEW.volume < 0 THEN
        RAISE EXCEPTION 'Volume cannot be negative';
    END IF;
    
    RETURN NEW;
END;
$$ language 'plpgsql';

CREATE TRIGGER validate_ohlcv_trigger BEFORE INSERT OR UPDATE ON market_data
    FOR EACH ROW EXECUTE FUNCTION validate_ohlcv_data();

-- Create function for calculating basic statistics
CREATE OR REPLACE FUNCTION calculate_symbol_stats(symbol_param VARCHAR(10))
RETURNS TABLE(
    avg_volume BIGINT,
    avg_price DECIMAL(12,4),
    price_volatility DECIMAL(8,4),
    data_points INTEGER
) AS $$
BEGIN
    RETURN QUERY
    SELECT 
        ROUND(AVG(volume))::BIGINT as avg_volume,
        AVG(close_price) as avg_price,
        STDDEV(close_price) / AVG(close_price) as price_volatility,
        COUNT(*) as data_points
    FROM market_data 
    WHERE symbol = symbol_param;
END;
$$ LANGUAGE plpgsql;