#pragma once

#include "ExecutionEngine_I.hpp"

class SimpleExecutionEngine : public ExecutionEngine_I {
public:
    std::optional<Fill>
    execute(const Order& order, const Candle& bar) override;
};
