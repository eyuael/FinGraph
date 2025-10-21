-- Migration 002: Add Backtest Tables
-- This migration adds tables for storing backtest results and related data
-- Version: 1.1.0
-- Date: 2025-10-20

-- Enable UUID extension if not already enabled
CREATE EXTENSION IF NOT EXISTS "uuid-ossp";

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

-- Create indexes for backtest tables
CREATE INDEX idx_backtest_results_symbol ON backtest_results(symbol);
CREATE INDEX idx_backtest_results_strategy ON backtest_results(strategy_name);
CREATE INDEX idx_backtest_results_created_at ON backtest_results(created_at DESC);
CREATE INDEX idx_trades_backtest_id ON trades(backtest_id);
CREATE INDEX idx_trades_symbol ON trades(symbol);
CREATE INDEX idx_trades_entry_date ON trades(entry_date DESC);
CREATE INDEX idx_performance_metrics_backtest_id ON performance_metrics(backtest_id);

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