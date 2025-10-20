# FinGraph

A comprehensive quantitative trading platform built with modern C++ and React, designed for backtesting trading strategies and visualizing financial data.

## Architecture

FinGraph is a modular system consisting of four main components:

### 🏗️ Core Components

- **Simulation Engine** (`simulation_engine/`) - C++20 backtesting engine with strategy framework
- **Web Server** (`web_server/`) - REST API built with Drogon framework
- **Frontend** (`fingraph_frontend/`) - React/Next.js dashboard with interactive charts
- **Data Ingestion** (`data_ingestion/`) - Market data collection and storage service

### 📊 Features

- **Strategy Backtesting**: Test moving average and RSI-based strategies
- **Performance Analytics**: Calculate Sharpe ratio, max drawdown, win rate, and total returns
- **Interactive Visualization**: Real-time charts using Plotly.js
- **RESTful API**: Complete HTTP interface for running and managing backtests
- **Modern Tech Stack**: C++20, React 19, Next.js 15, TailwindCSS

## Quick Start

### Prerequisites

- C++20 compatible compiler (Clang, GCC, or MSVC)
- CMake 3.16+
- Node.js 18+
- PostgreSQL (optional, for data persistence)
- Drogon framework
- nlohmann/json library

### Installation

1. **Clone the repository**
   ```bash
   git clone <repository-url>
   cd FinGraph
   ```

2. **Build the Simulation Engine**
   ```bash
   cd simulation_engine
   mkdir build && cd build
   cmake ..
   make
   ```

3. **Build the Web Server**
   ```bash
   cd ../../web_server
   mkdir build && cd build
   cmake ..
   make
   ```

4. **Install Frontend Dependencies**
   ```bash
   cd ../../fingraph_frontend
   npm install
   ```

### Running the Application

1. **Start the Web Server**
   ```bash
   cd web_server/build
   ./FinGraphWebServer
   ```

2. **Start the Frontend**
   ```bash
   cd fingraph_frontend
   npm run dev
   ```

3. **Access the Dashboard**
   Open [http://localhost:3000](http://localhost:3000) in your browser

## API Endpoints

### Backtesting
- `POST /api/v1/backtest` - Run a new backtest
- `GET /api/v1/backtest/{id}` - Get backtest results
- `GET /api/v1/backtest` - List all backtests

### Data Management
- `POST /api/v1/data/upload` - Upload market data
- `GET /api/v1/data/symbols` - List available symbols
- `GET /api/v1/data/{symbol}` - Get historical data

### Strategies
- `GET /api/v1/strategies` - List available strategies
- `GET /api/v1/strategies/{name}` - Get strategy parameters

## Project Structure

```
FinGraph/
├── simulation_engine/          # C++ backtesting engine
│   ├── include/fingraph/       # Header files
│   ├── src/                    # Implementation files
│   ├── src/strategies/         # Trading strategies
│   └── tests/                  # Unit tests
├── web_server/                 # C++ REST API
│   ├── src/controllers/        # HTTP controllers
│   ├── src/services/           # Business logic
│   └── src/utils/              # Utilities
├── fingraph_frontend/          # React dashboard
│   ├── src/app/                # Next.js pages
│   ├── src/components/          # React components
│   └── src/lib/                # API client
└── data_ingestion/             # Data collection service
    ├── include/fingraph/       # Headers
    └── src/                    # Implementation
```

## Available Strategies

### Moving Average Strategy
- **Parameters**: Short window, Long window
- **Logic**: Buy when short MA crosses above long MA, sell when crossing below

### RSI Strategy
- **Parameters**: RSI period, Overbought threshold, Oversold threshold
- **Logic**: Buy when RSI is oversold, sell when overbought

## Development

### Adding New Strategies

1. Create header file in `simulation_engine/include/fingraph/strategies/`
2. Implement strategy in `simulation_engine/src/strategies/`
3. Register strategy in `BacktestEngine::initializeStrategies()`
4. Update API to expose new strategy parameters

### Running Tests

```bash
cd simulation_engine/build
ctest
```

### Code Style

- C++: Follow Google C++ Style Guide
- React/TypeScript: ESLint configuration provided
- Use clangd for IDE support

## Performance

- **Backtesting Speed**: ~1M data points per second
- **Memory Usage**: <100MB for typical datasets
- **API Response Time**: <500ms for standard requests

## License

This project is licensed under the MIT License - see the LICENSE file for details.

## Contributing

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Add tests for new functionality
5. Submit a pull request

## Support

For questions and support, please open an issue on the GitHub repository.