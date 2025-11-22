#pragma once

#include "Types.h"
#include <cstddef>
#include <vector>
#include <memory>

class MarketDataFeed_I
{
public:
  virtual ~MarketDataFeed_I() = default;

  // Is there another bar available?
  virtual bool hasNext() const = 0;

  // Return the next bar and advance the internal index.
  virtual const Candle &next() = 0;

  // Index of the most recently returned bar.
  virtual std::size_t currentIndex() const = 0;
};

// Factory for a simple in-memory feed backed by std::vector<Candle>.
std::unique_ptr<MarketDataFeed_I> makeVectorFeed(std::vector<Candle> candles);
