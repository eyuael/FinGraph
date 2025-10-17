#include "fingraph/Strategy.h"
#include <vector>

namespace fingraph {

/**
 * @class RSIStrategy
 * @brief Implements an RSI (Relative Strength Index) mean-reversion strategy.
 *
 * The RSI is a momentum indicator that measures the speed and change of price movements.
 * This strategy identifies overbought and oversold conditions. It generates a BUY signal
 * when the RSI crosses up through the oversold threshold (e.g., 30) and a SELL signal
 * when it crosses down through the overbought threshold (e.g., 70).
 */
class RSIStrategy : public Strategy {
public:
    /**
     * @brief Constructor for RSIStrategy.
     * Initializes with default RSI period and thresholds.
     */
    RSIStrategy();

    /**
     * @brief Default destructor.
     */
    ~RSIStrategy() override = default;

    /**
     * @brief Initializes the strategy with historical market data.
     *
     * This method pre-calculates the RSI values for the entire dataset to optimize
     * the performance of the backtest simulation loop.
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
     *               Expected keys: "period", "oversoldThreshold", "overboughtThreshold".
     */
    void updateParameters(const std::map<std::string, double>& params) override;

private:
    size_t period_;                 ///< The lookback period for RSI calculation (typically 14).
    double oversoldThreshold_;      ///< The RSI level considered oversold (e.g., 30.0).
    double overboughtThreshold_;    ///< The RSI level considered overbought (e.g., 70.0).
    std::vector<double> rsiValues_; ///< Pre-calculated RSI values for each data point.

    /**
     * @brief Calculates the RSI values for the entire dataset.
     *
     * The calculation involves first determining average gains and losses over the period,
     * then smoothing them, and finally computing the RSI.
     *
     * @param data The OHLCV data to use for the calculation.
     */
    void calculateRSI(const std::vector<OHLCV>& data);
};

} // namespace fingraph