-- Migration 001: Initial Schema
-- This migration creates the initial database structure for FinGraph
-- Version: 1.0.0
-- Date: 2025-10-20

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

-- Create indexes for performance
CREATE INDEX idx_market_data_symbol_timestamp ON market_data(symbol, timestamp DESC);
CREATE INDEX idx_market_data_timestamp ON market_data(timestamp DESC);
CREATE INDEX idx_market_data_symbol ON market_data(symbol);
CREATE INDEX idx_symbols_active ON symbols(is_active);
CREATE INDEX idx_api_metadata_symbol ON api_metadata(symbol);
CREATE INDEX idx_api_metadata_last_refreshed ON api_metadata(last_refreshed DESC);
CREATE INDEX idx_data_ingestion_logs_symbol ON data_ingestion_logs(symbol);
CREATE INDEX idx_data_ingestion_logs_created_at ON data_ingestion_logs(created_at DESC);

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

-- Create function for data validation
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