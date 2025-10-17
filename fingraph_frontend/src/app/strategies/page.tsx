import Link from 'next/link';
import { fetchStrategies } from '@/lib/api';

export default async function StrategiesPage() {
  const strategies = await fetchStrategies();

  return (
    <div className="min-h-screen bg-gray-50">
      <header className="bg-white shadow-sm border-b">
        <div className="max-w-7xl mx-auto px-4 sm:px-6 lg:px-8">
          <div className="flex justify-between items-center py-6">
            <div>
              <Link href="/" className="text-blue-600 hover:text-blue-800">← Back to Dashboard</Link>
              <h1 className="text-3xl font-bold text-gray-900 mt-2">Trading Strategies</h1>
            </div>
            <button className="bg-blue-600 text-white px-4 py-2 rounded hover:bg-blue-700">
              New Strategy
            </button>
          </div>
        </div>
      </header>

      <main className="max-w-7xl mx-auto px-4 sm:px-6 lg:px-8 py-8">
        <div className="grid grid-cols-1 md:grid-cols-2 lg:grid-cols-3 gap-6">
          {strategies.map((strategy) => (
            <div key={strategy.id} className="bg-white rounded-lg shadow p-6 hover:shadow-lg transition-shadow">
              <h3 className="text-xl font-semibold text-gray-900 mb-2">{strategy.name}</h3>
              <p className="text-gray-600 mb-4">{strategy.description}</p>

              <div className="grid grid-cols-2 gap-4 mb-4 text-sm">
                <div>
                  <span className="text-gray-500">Return:</span>
                  <span className={`ml-1 font-medium ${strategy.metrics.totalReturn >= 0 ? 'text-green-600' : 'text-red-600'}`}>
                    {strategy.metrics.totalReturn.toFixed(1)}%
                  </span>
                </div>
                <div>
                  <span className="text-gray-500">Trades:</span>
                  <span className="ml-1 font-medium">{strategy.metrics.totalTrades}</span>
                </div>
              </div>

              <Link
                href={`/strategy/${strategy.id}`}
                className="inline-block w-full text-center bg-blue-600 text-white px-4 py-2 rounded hover:bg-blue-700 transition-colors"
              >
                View Results
              </Link>
            </div>
          ))}
        </div>

        {strategies.length === 0 && (
          <div className="text-center py-12">
            <p className="text-gray-500 text-lg">No strategies found.</p>
            <p className="text-gray-400 mt-2">Run your first backtest to get started.</p>
          </div>
        )}
      </main>
    </div>
  );
}