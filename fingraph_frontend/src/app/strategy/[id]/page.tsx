import { notFound } from 'next/navigation';
import Link from 'next/link';
import PriceChart from '@/components/PriceChart';
import EquityChart from '@/components/EquityChart';
import PerformanceMetrics from '@/components/PerformanceMetrics';
import { fetchStrategyData } from '@/lib/api';

interface StrategyPageProps {
  params: Promise<{ id: string }>;
}

export default async function StrategyPage({ params }: StrategyPageProps) {
  const { id } = await params;
  const strategy = await fetchStrategyData(id);

  if (!strategy) {
    notFound();
  }

  return (
    <div className="min-h-screen bg-gray-50">
      <header className="bg-white shadow-sm border-b">
        <div className="max-w-7xl mx-auto px-4 sm:px-6 lg:px-8">
          <div className="flex justify-between items-center py-6">
            <div>
              <Link href="/" className="text-blue-600 hover:text-blue-800">← Back to Dashboard</Link>
              <h1 className="text-3xl font-bold text-gray-900 mt-2">{strategy.name}</h1>
              <p className="text-gray-600">{strategy.description}</p>
            </div>
          </div>
        </div>
      </header>

      <main className="max-w-7xl mx-auto px-4 sm:px-6 lg:px-8 py-8">
        <div className="grid grid-cols-1 lg:grid-cols-3 gap-8">
          <div className="lg:col-span-2 space-y-8">
            <div className="bg-white rounded-lg shadow p-6">
              <h3 className="text-xl font-semibold text-gray-900 mb-4">Price Chart</h3>
              <PriceChart data={strategy.priceData} trades={strategy.trades} />
            </div>

            <div className="bg-white rounded-lg shadow p-6">
              <h3 className="text-xl font-semibold text-gray-900 mb-4">Equity Curve</h3>
              <EquityChart data={strategy.equityData} />
            </div>
          </div>

          <div className="space-y-8">
            <PerformanceMetrics metrics={strategy.metrics} />

            <div className="bg-white rounded-lg shadow p-6">
              <h3 className="text-lg font-semibold text-gray-900 mb-4">Recent Trades</h3>
              <div className="space-y-2">
                {strategy.trades.map((trade, index) => (
                  <div key={index} className="flex justify-between items-center p-2 bg-gray-50 rounded">
                    <div>
                      <div className="font-medium">{trade.type.toUpperCase()}</div>
                      <div className="text-sm text-gray-600">{trade.date}</div>
                    </div>
                    <div className="text-right">
                      <div className="font-medium">${trade.price}</div>
                      <div className="text-sm text-gray-600">{trade.quantity} shares</div>
                    </div>
                  </div>
                ))}
              </div>
            </div>
          </div>
        </div>
      </main>
    </div>
  );
}