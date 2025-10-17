import Link from "next/link";

export default function Dashboard() {
  return (
    <div className="min-h-screen bg-gray-50">
      <header className="bg-white shadow-sm border-b">
        <div className="max-w-7xl mx-auto px-4 sm:px-6 lg:px-8">
          <div className="flex justify-between items-center py-6">
            <h1 className="text-3xl font-bold text-gray-900">FinGraph Dashboard</h1>
            <nav className="space-x-4">
              <Link href="/" className="text-blue-600 hover:text-blue-800">Dashboard</Link>
              <Link href="/strategies" className="text-gray-600 hover:text-gray-800">Strategies</Link>
            </nav>
          </div>
        </div>
      </header>

      <main className="max-w-7xl mx-auto px-4 sm:px-6 lg:px-8 py-8">
        <div className="grid grid-cols-1 md:grid-cols-2 lg:grid-cols-3 gap-6">
          <div className="bg-white rounded-lg shadow p-6">
            <h3 className="text-lg font-semibold text-gray-900 mb-2">Recent Backtests</h3>
            <p className="text-gray-600">View your latest trading simulation results</p>
            <Link href="/strategies" className="mt-4 inline-block text-blue-600 hover:text-blue-800">
              View Strategies →
            </Link>
          </div>

          <div className="bg-white rounded-lg shadow p-6">
            <h3 className="text-lg font-semibold text-gray-900 mb-2">Performance Analytics</h3>
            <p className="text-gray-600">Analyze trading performance metrics</p>
            <Link href="/analytics" className="mt-4 inline-block text-blue-600 hover:text-blue-800">
              View Analytics →
            </Link>
          </div>

          <div className="bg-white rounded-lg shadow p-6">
            <h3 className="text-lg font-semibold text-gray-900 mb-2">Run New Backtest</h3>
            <p className="text-gray-600">Execute new trading strategy simulations</p>
            <button className="mt-4 bg-blue-600 text-white px-4 py-2 rounded hover:bg-blue-700">
              Start Backtest
            </button>
          </div>
        </div>

        <div className="mt-8 bg-white rounded-lg shadow p-6">
          <h3 className="text-xl font-semibold text-gray-900 mb-4">Quick Overview</h3>
          <div className="grid grid-cols-1 md:grid-cols-4 gap-4">
            <div className="text-center">
              <div className="text-2xl font-bold text-green-600">+12.5%</div>
              <div className="text-sm text-gray-600">Total Return</div>
            </div>
            <div className="text-center">
              <div className="text-2xl font-bold text-blue-600">156</div>
              <div className="text-sm text-gray-600">Total Trades</div>
            </div>
            <div className="text-center">
              <div className="text-2xl font-bold text-purple-600">1.25</div>
              <div className="text-sm text-gray-600">Sharpe Ratio</div>
            </div>
            <div className="text-center">
              <div className="text-2xl font-bold text-red-600">8.2%</div>
              <div className="text-sm text-gray-600">Max Drawdown</div>
            </div>
          </div>
        </div>
      </main>
    </div>
  );
}
