#pragma once

#include "TradingTypes.hpp"

class DataFeed_I
{
public:
  virtual ~DataFeed_I() = default;

  virtual bool hasNext() const = 0;
  virtual const Candle &next() = 0;
};
