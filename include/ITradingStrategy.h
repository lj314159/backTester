#pragma once

#include "Types.h"

#include <cstddef>
#include <memory>
#include <string>

class BacktestEngine;

/*
 * ITradingStrategy
 * ----------------
 * Interface for all trading strategies.
 *
 *  - onStart: called once before the backtest loop.
 *  - onBar:   called once per bar with the current Candle.
 *  - onEnd:   called once after the backtest loop finishes.
 */
class ITradingStrategy
{
public:
  virtual ~ITradingStrategy() = default;

  virtual void onStart(BacktestEngine &engine) = 0;

  virtual void onBar(std::size_t index,
                     const Candle &bar,
                     BacktestEngine &engine) = 0;

  virtual void onEnd(BacktestEngine &engine) = 0;
};

/*
 * Factory functions for concrete strategies.
 * Implemented in their corresponding .cpp files.
 */

// Simple single-window SMA strategy.
std::unique_ptr<ITradingStrategy>
makeSimpleSMAStrategy(const std::string &symbol,
                      std::size_t window);

// Fast/slow SMA crossover strategy.
std::unique_ptr<ITradingStrategy>
makeSmaCrossoverStrategy(const std::string &symbol,
                         std::size_t fastWindow,
                         std::size_t slowWindow);

// RSI mean-reversion strategy.
std::unique_ptr<ITradingStrategy>
makeRsiReversionStrategy(const std::string &symbol,
                         std::size_t period,
                         double overbought,
                         double oversold);

// Breakout of recent high/low range.
std::unique_ptr<ITradingStrategy>
makeBreakoutStrategy(const std::string &symbol,
                     std::size_t lookbackWindow);
