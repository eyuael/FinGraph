#include "fingraph/Strategy.h"
#include <vector>

namespace fingraph {

/**
 * @class MovingAverageStrategy
 * @brief Implements a moving average crossover trading strategy.
 *
 * This strategy uses two simple moving averages (SMAs): a short-period and a long-period.
 * It generates a BUY signal when the short SMA crosses above the long SMA,
 * indicating potential upward momentum. It generates a SELL signal when the short
 * SMA crosses below the long SMA, indicating potential downward momentum.
 */
class MovingAverageStrategy : public Strategy {
public:
    /**
     * @brief Constructor for MovingAverageStrategy.
     * Initializes with default short and long periods.
     */
    MovingAverageStrategy();

    /**
     * @brief Default destructor.
     */
    ~MovingAverageStrategy() override = default;

    /**
     * @brief Initializes the strategy with historical market data.
     *
     * This method pre-calculates the short and long moving averages for the entire
     * dataset to ensure that the generateSignal method is fast during the backtest.
     *
     * @param data A vector of OHLCV data points.
     */
    void initialize(const std::vector<OHLCV>& data) override;

    /**
     * @brief Generates a trading signal for a specific point in time.
     *
     * @param index The index in the original data vector for which to generate a signal.
     * @return A Signal enum value (BUY, SELL, or NONE).
     */
    Signal generateSignal(size_t index) const override;

    /**
     * @brief Updates the strategy's parameters.
     *
     * @param params A map containing parameter names and their new values.
     *               Expected keys: "shortPeriod", "longPeriod".
     */
    void updateParameters(const std::map<std::string, double>& params) override;

private:
    size_t shortPeriod_;         ///< The period for the short-term moving average.
    size_t longPeriod_;          ///< The period for the long-term moving average.
    std::vector<double> shortMA_; ///< Pre-calculated values of the short moving average.
    std::vector<double> longMA_;  ///< Pre-calculated values of the long moving average.

    /**
     * @brief Calculates the simple moving averages for the entire dataset.
     * @param data The OHLCV data to use for the calculation.
     */
    void calculateMovingAverages(const std::vector<OHLCV>& data);
};

} // namespace fingraph