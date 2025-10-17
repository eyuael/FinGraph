#pragma once
#include "fingraph/MarketData.h"
#include <vector>
#include <string>
#include <map>

namespace fingraph {

enum class Signal {
    NONE,
    BUY,
    SELL
};

class Strategy {
public:
    Strategy(const std::string& name) : name_(name) {}
    virtual ~Strategy() = default;
    
    virtual void initialize(const std::vector<OHLCV>& data) = 0;
    virtual Signal generateSignal(size_t index) const = 0;
    virtual void updateParameters(const std::map<std::string, double>& params) = 0;
    
    const std::string& getName() const { return name_; }
    
protected:
    std::string name_;
};

} // namespace fingraph