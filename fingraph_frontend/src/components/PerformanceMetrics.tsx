interface PerformanceMetricsProps {
  metrics: {
    totalReturn: number;
    sharpeRatio: number;
    maxDrawdown: number;
    winRate: number;
    totalTrades: number;
  };
}

export default function PerformanceMetrics({ metrics }: PerformanceMetricsProps) {
  const formatPercent = (value: number) => `${value.toFixed(2)}%`;
  const formatNumber = (value: number) => value.toFixed(2);

  return (
    <div className="bg-white rounded-lg shadow p-6">
      <h3 className="text-lg font-semibold text-gray-900 mb-4">Performance Metrics</h3>
      <div className="space-y-4">
        <div className="flex justify-between items-center">
          <span className="text-gray-600">Total Return</span>
          <span className={`font-semibold ${metrics.totalReturn >= 0 ? 'text-green-600' : 'text-red-600'}`}>
            {formatPercent(metrics.totalReturn)}
          </span>
        </div>

        <div className="flex justify-between items-center">
          <span className="text-gray-600">Sharpe Ratio</span>
          <span className="font-semibold text-blue-600">{formatNumber(metrics.sharpeRatio)}</span>
        </div>

        <div className="flex justify-between items-center">
          <span className="text-gray-600">Max Drawdown</span>
          <span className="font-semibold text-red-600">{formatPercent(metrics.maxDrawdown)}</span>
        </div>

        <div className="flex justify-between items-center">
          <span className="text-gray-600">Win Rate</span>
          <span className="font-semibold text-green-600">{formatPercent(metrics.winRate)}</span>
        </div>

        <div className="flex justify-between items-center">
          <span className="text-gray-600">Total Trades</span>
          <span className="font-semibold text-gray-900">{metrics.totalTrades}</span>
        </div>
      </div>
    </div>
  );
}