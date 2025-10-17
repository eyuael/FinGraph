// Placeholder functions for C++ REST API integration
// Replace with actual API calls when the C++ backend is implemented

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

// Placeholder: Fetch strategy data from C++ REST API
export async function fetchStrategyData(strategyId: string): Promise<StrategyData | null> {
  try {
    // TODO: Replace with actual API call
    // const response = await fetch(`http://localhost:8080/api/strategies/${strategyId}`);
    // if (!response.ok) return null;
    // return await response.json();

    // Mock data for now
    return {
      id: strategyId,
      name: `Strategy ${strategyId}`,
      description: 'Sample trading strategy backtest results',
      priceData: {
        dates: Array.from({ length: 100 }, (_, i) => {
          const date = new Date();
          date.setDate(date.getDate() - (99 - i));
          return date.toISOString().split('T')[0];
        }),
        prices: Array.from({ length: 100 }, (_, i) => 100 + Math.sin(i * 0.1) * 10 + Math.random() * 5)
      },
      equityData: {
        dates: Array.from({ length: 100 }, (_, i) => {
          const date = new Date();
          date.setDate(date.getDate() - (99 - i));
          return date.toISOString().split('T')[0];
        }),
        equity: Array.from({ length: 100 }, (_, i) => 10000 + i * 50 + Math.sin(i * 0.1) * 200)
      },
      trades: [
        { date: '2024-01-15', type: 'buy', price: 98.5, quantity: 100 },
        { date: '2024-02-01', type: 'sell', price: 105.2, quantity: 100 },
        { date: '2024-02-15', type: 'buy', price: 102.1, quantity: 80 },
        { date: '2024-03-01', type: 'sell', price: 108.7, quantity: 80 },
      ],
      metrics: {
        totalReturn: 8.7,
        sharpeRatio: 1.45,
        maxDrawdown: 3.2,
        winRate: 68.5,
        totalTrades: 24
      }
    };
  } catch (error) {
    console.error('Error fetching strategy data:', error);
    return null;
  }
}

// Placeholder: Run backtest via C++ REST API
export async function runBacktest(request: BacktestRequest): Promise<string> {
  try {
    // TODO: Replace with actual API call
    // const response = await fetch('http://localhost:8080/api/backtest', {
    //   method: 'POST',
    //   headers: { 'Content-Type': 'application/json' },
    //   body: JSON.stringify(request)
    // });
    // const result = await response.json();
    // return result.strategyId;

    // Mock response for now
    console.log('Running backtest with parameters:', request);
    return `strategy_${Date.now()}`;
  } catch (error) {
    console.error('Error running backtest:', error);
    throw new Error('Failed to run backtest');
  }
}

// Placeholder: Get list of available strategies
export async function fetchStrategies(): Promise<StrategyData[]> {
  try {
    // TODO: Replace with actual API call
    // const response = await fetch('http://localhost:8080/api/strategies');
    // return await response.json();

    // Mock data for now
    return [
      {
        id: '1',
        name: 'Moving Average Crossover',
        description: 'Simple MA crossover strategy',
        priceData: { dates: [], prices: [] },
        equityData: { dates: [], equity: [] },
        trades: [],
        metrics: { totalReturn: 0, sharpeRatio: 0, maxDrawdown: 0, winRate: 0, totalTrades: 0 }
      },
      {
        id: '2',
        name: 'RSI Mean Reversion',
        description: 'RSI-based mean reversion strategy',
        priceData: { dates: [], prices: [] },
        equityData: { dates: [], equity: [] },
        trades: [],
        metrics: { totalReturn: 0, sharpeRatio: 0, maxDrawdown: 0, winRate: 0, totalTrades: 0 }
      }
    ];
  } catch (error) {
    console.error('Error fetching strategies:', error);
    return [];
  }
}