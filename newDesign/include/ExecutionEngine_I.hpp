#pragma once

#include <optional>
#include "TradingTypes.hpp"

class ExecutionEngine_I {
public:
    virtual ~ExecutionEngine_I() = default;

    virtual std::optional<Fill>
    execute(const Order& order, const Candle& bar) = 0;
};
