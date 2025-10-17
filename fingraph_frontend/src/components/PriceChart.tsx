'use client';

import dynamic from 'next/dynamic';
import { useEffect, useState } from 'react';

const Plot = dynamic(() => import('react-plotly.js'), { ssr: false });

interface PriceChartProps {
  data: {
    dates: string[];
    prices: number[];
  };
  trades: Array<{
    date: string;
    type: 'buy' | 'sell';
    price: number;
    quantity: number;
  }>;
}

export default function PriceChart({ data, trades }: PriceChartProps) {
  const [isClient, setIsClient] = useState(false);

  useEffect(() => {
    setIsClient(true);
  }, []);

  if (!isClient) {
    return <div className="h-96 bg-gray-100 rounded animate-pulse"></div>;
  }

  const priceTrace = {
    x: data.dates,
    y: data.prices,
    type: 'scatter' as const,
    mode: 'lines' as const,
    name: 'Price',
    line: { color: '#2563eb' }
  };

  const buyTrades = trades.filter(t => t.type === 'buy');
  const sellTrades = trades.filter(t => t.type === 'sell');

  const buyTrace = {
    x: buyTrades.map(t => t.date),
    y: buyTrades.map(t => t.price),
    type: 'scatter' as const,
    mode: 'markers' as const,
    name: 'Buy',
    marker: { color: '#16a34a', size: 10, symbol: 'triangle-up' }
  };

  const sellTrace = {
    x: sellTrades.map(t => t.date),
    y: sellTrades.map(t => t.price),
    type: 'scatter' as const,
    mode: 'markers' as const,
    name: 'Sell',
    marker: { color: '#dc2626', size: 10, symbol: 'triangle-down' }
  };

  return (
    <div className="w-full">
      <Plot
        data={[priceTrace, buyTrace, sellTrace]}
        layout={{
          height: 400,
          margin: { t: 10, r: 10, b: 40, l: 60 },
          xaxis: { title: 'Date' },
          yaxis: { title: 'Price ($)' },
          showlegend: true
        }}
        config={{ responsive: true }}
        style={{ width: '100%' }}
      />
    </div>
  );
}