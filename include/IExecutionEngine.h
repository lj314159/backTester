#pragma once

#include "Types.h"
#include <vector>

class Portfolio;

class IExecutionEngine {
public:
    virtual ~IExecutionEngine() = default;

    // Fill all pending orders given the current bar, updating the portfolio.
    virtual void execute(const std::vector<Order>& orders,
                         const Candle& bar,
                         Portfolio& portfolio) = 0;
};
