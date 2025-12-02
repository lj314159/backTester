#pragma once

#include <cstddef>
#include "TradingTypes.hpp"

class BacktestEngine;

class Strategy_I
{
public:
  virtual ~Strategy_I() = default;

  virtual void onStart(BacktestEngine &engine) = 0;
  virtual void onBar(std::size_t index,
                     const Candle &bar,
                     BacktestEngine &engine)
    = 0;
  virtual void onEnd(BacktestEngine &engine) = 0;
};
