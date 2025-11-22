#pragma once

#include "Types.h"

#include <cstddef>
#include <memory>
#include <string>

class BacktestEngine;

/*
 * TradingStrategy_I
 * ----------------
 * Interface for all trading strategies.
 *
 *  - onStart: called once before the backtest loop.
 *  - onBar:   called once per bar with the current Candle.
 *  - onEnd:   called once after the backtest loop finishes.
 */
class TradingStrategy_I
{
public:
  virtual ~TradingStrategy_I() = default;

  virtual void onStart(BacktestEngine &engine) = 0;

  virtual void onBar(std::size_t index,
                     const Candle &bar,
                     BacktestEngine &engine)
    = 0;

  virtual void onEnd(BacktestEngine &engine) = 0;
};

/*
 * Factory functions for concrete strategies.
 * Implemented in their corresponding .cpp files.
 */

// Simple single-window SMA strategy.
std::unique_ptr<TradingStrategy_I>
makeSimpleSMAStrategy(const std::string &symbol,
                      std::size_t window);

// Fast/slow SMA crossover strategy.
std::unique_ptr<TradingStrategy_I>
makeSmaCrossoverStrategy(const std::string &symbol,
                         std::size_t fastWindow,
                         std::size_t slowWindow);

// RSI mean-reversion strategy.
std::unique_ptr<TradingStrategy_I>
makeRsiReversionStrategy(const std::string &symbol,
                         std::size_t period,
                         double overbought,
                         double oversold);

// Breakout of recent high/low range.
std::unique_ptr<TradingStrategy_I>
makeBreakoutStrategy(const std::string &symbol,
                     std::size_t lookbackWindow);

// Breakout of recent high/low range.
std::unique_ptr<TradingStrategy_I>
makeBreakoutStrategy(const std::string &symbol,
                     std::size_t lookbackWindow);

// Trend + RSI dip-buy strategy.
std::unique_ptr<TradingStrategy_I>
makeTrendRsiStrategy(const std::string &symbol,
                     std::size_t rsiPeriod,
                     double overbought,
                     double oversold,
                     std::size_t trendWindow);
