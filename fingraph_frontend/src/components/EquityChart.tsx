'use client';

import dynamic from 'next/dynamic';
import { useEffect, useState } from 'react';

const Plot = dynamic(() => import('react-plotly.js'), { ssr: false });

interface EquityChartProps {
  data: {
    dates: string[];
    equity: number[];
  };
}

export default function EquityChart({ data }: EquityChartProps) {
  const [isClient, setIsClient] = useState(false);

  useEffect(() => {
    setIsClient(true);
  }, []);

  if (!isClient) {
    return <div className="h-96 bg-gray-100 rounded animate-pulse"></div>;
  }

  const equityTrace = {
    x: data.dates,
    y: data.equity,
    type: 'scatter' as const,
    mode: 'lines' as const,
    name: 'Equity',
    line: { color: '#16a34a' },
    fill: 'tozeroy',
    fillcolor: 'rgba(22, 163, 74, 0.1)'
  };

  return (
    <div className="w-full">
      <Plot
        data={[equityTrace]}
        layout={{
          height: 400,
          margin: { t: 10, r: 10, b: 40, l: 60 },
          xaxis: { title: 'Date' },
          yaxis: { title: 'Equity ($)' },
          showlegend: false
        }}
        config={{ responsive: true }}
        style={{ width: '100%' }}
      />
    </div>
  );
}