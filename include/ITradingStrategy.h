#pragma once

#include "Types.h"
#include <cstddef>
#include <memory>
#include <string>

// Forward declaration
class BacktestEngine;

/*
 * ITradingStrategy
 * ----------------
 * This is the Strategy interface in the Strategy pattern.
 * Concrete strategy classes (like SimpleSMAStrategy) implement this.
 */
class ITradingStrategy {
public:
    virtual ~ITradingStrategy() = default;

    // Called once before the first bar
    virtual void onStart(BacktestEngine& engine) = 0;

    // Called once per bar (index and the current Candle are passed)
    virtual void onBar(std::size_t index,
                       const Candle& bar,
                       BacktestEngine& engine) = 0;

    // Called once after the last bar
    virtual void onEnd(BacktestEngine& engine) = 0;
};

/*
 * Factory function for building a Simple SMA Strategy.
 * ----------------------------------------------------
 * Defined in SimpleSMAStrategy.cpp, but declared here
 * so that main.cpp and BacktestEngine can call it.
 */
std::unique_ptr<ITradingStrategy>
makeSimpleSMAStrategy(const std::string& symbol, std::size_t window);
