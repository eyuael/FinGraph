// API functions for C++ REST API integration
// Base URL for the backend API
const API_BASE_URL = process.env.NEXT_PUBLIC_API_URL || 'http://localhost:8080';

export interface StrategyData {
  id: string;
  name: string;
  description: string;
  priceData: {
    dates: string[];
    prices: number[];
  };
  equityData: {
    dates: string[];
    equity: number[];
  };
  trades: Array<{
    date: string;
    type: 'buy' | 'sell';
    price: number;
    quantity: number;
  }>;
  metrics: {
    totalReturn: number;
    sharpeRatio: number;
    maxDrawdown: number;
    winRate: number;
    totalTrades: number;
  };
}

export interface BacktestRequest {
  strategyId: string;
  parameters: Record<string, string | number | boolean>;
  startDate: string;
  endDate: string;
}

export interface DataInfo {
  id: string;
  filename: string;
  size: number;
  rows: number;
  lastModified: number;
  dateRange: {
    start: string;
    end: string;
  };
}

export interface DataPreview {
  headers: string[];
  rows: string[][];
  totalRows: number;
}

// Fetch strategy data from C++ REST API
export async function fetchStrategyData(strategyId: string): Promise<StrategyData | null> {
  try {
    const response = await fetch(`${API_BASE_URL}/api/v1/backtest/${strategyId}`);
    if (!response.ok) return null;
    const data = await response.json();
    
    // Transform backend response to frontend format
    return {
      id: strategyId,
      name: data.strategy || `Strategy ${strategyId}`,
      description: 'Trading strategy backtest results',
      priceData: {
        dates: data.priceData?.dates || [],
        prices: data.priceData?.prices || []
      },
      equityData: {
        dates: data.equityData?.dates || [],
        equity: data.equityData?.equity || []
      },
      trades: data.trades || [],
      metrics: data.metrics || {
        totalReturn: 0,
        sharpeRatio: 0,
        maxDrawdown: 0,
        winRate: 0,
        totalTrades: 0
      }
    };
  } catch (error) {
    console.error('Error fetching strategy data:', error);
    return null;
  }
}

// Run backtest via C++ REST API
export async function runBacktest(request: BacktestRequest): Promise<string> {
  try {
    const response = await fetch(`${API_BASE_URL}/api/v1/backtest`, {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({
        dataId: 'default_data', // This should come from data selection
        strategy: request.strategyId,
        initialCash: 10000,
        parameters: request.parameters
      })
    });
    
    if (!response.ok) {
      throw new Error('Failed to run backtest');
    }
    
    const result = await response.json();
    return result.strategyId || result.id || `backtest_${Date.now()}`;
  } catch (error) {
    console.error('Error running backtest:', error);
    throw new Error('Failed to run backtest');
  }
}

// Get list of available strategies
export async function fetchStrategies(): Promise<StrategyData[]> {
  try {
    const response = await fetch(`${API_BASE_URL}/api/v1/strategies`);
    if (!response.ok) return [];
    
    const strategies = await response.json();
    
    // Transform backend response to frontend format
    return strategies.map((strategy: any) => ({
      id: strategy.id,
      name: strategy.name,
      description: strategy.description,
      priceData: { dates: [], prices: [] },
      equityData: { dates: [], equity: [] },
      trades: [],
      metrics: { totalReturn: 0, sharpeRatio: 0, maxDrawdown: 0, winRate: 0, totalTrades: 0 }
    }));
  } catch (error) {
    console.error('Error fetching strategies:', error);
    return [];
  }
}

// Get backtest results by ID
export async function fetchBacktestResults(backtestId: string): Promise<any> {
  try {
    const response = await fetch(`${API_BASE_URL}/api/v1/backtest/${backtestId}`);
    if (!response.ok) return null;
    return await response.json();
  } catch (error) {
    console.error('Error fetching backtest results:', error);
    return null;
  }
}

// List all backtests
export async function fetchBacktestList(): Promise<any[]> {
  try {
    const response = await fetch(`${API_BASE_URL}/api/v1/backtest`);
    if (!response.ok) return [];
    return await response.json();
  } catch (error) {
    console.error('Error fetching backtest list:', error);
    return [];
  }
}

// List available data files
export async function fetchDataList(): Promise<string[]> {
  try {
    const response = await fetch(`${API_BASE_URL}/api/v1/data/list`);
    if (!response.ok) return [];
    return await response.json();
  } catch (error) {
    console.error('Error fetching data list:', error);
    return [];
  }
}

// Get data metadata
export async function fetchDataMetadata(dataId: string): Promise<DataInfo | null> {
  try {
    const response = await fetch(`${API_BASE_URL}/api/v1/data/${dataId}/metadata`);
    if (!response.ok) return null;
    return await response.json();
  } catch (error) {
    console.error('Error fetching data metadata:', error);
    return null;
  }
}

// Preview data file
export async function fetchDataPreview(dataId: string): Promise<DataPreview | null> {
  try {
    const response = await fetch(`${API_BASE_URL}/api/v1/data/${dataId}/preview`);
    if (!response.ok) return null;
    return await response.json();
  } catch (error) {
    console.error('Error fetching data preview:', error);
    return null;
  }
}

// Delete data file
export async function deleteData(dataId: string): Promise<boolean> {
  try {
    const response = await fetch(`${API_BASE_URL}/api/v1/data/${dataId}`, {
      method: 'DELETE'
    });
    return response.ok;
  } catch (error) {
    console.error('Error deleting data:', error);
    return false;
  }
}

// Upload data file
export async function uploadData(file: File): Promise<string | null> {
  try {
    const formData = new FormData();
    formData.append('file', file);
    
    const response = await fetch(`${API_BASE_URL}/api/v1/data/upload`, {
      method: 'POST',
      body: formData
    });
    
    if (!response.ok) return null;
    const result = await response.json();
    return result.dataId;
  } catch (error) {
    console.error('Error uploading data:', error);
    return null;
  }
}