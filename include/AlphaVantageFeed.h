#pragma once

#include "IMarketDataFeed.h"
#include "Types.h"

#include <cstddef>
#include <memory>
#include <string>
#include <vector>

class AlphaVantageFeed : public IMarketDataFeed
{
public:
  explicit AlphaVantageFeed(std::vector<Candle> candles);

  bool hasNext() const override;
  const Candle &next() override;
  std::size_t currentIndex() const override;

private:
  std::vector<Candle> candles_;
  std::size_t index_;
};

// Factory: builds an AlphaVantageFeed by fetching data over HTTP.
// lookbackBars > 0  -> keep only the most recent lookbackBars candles
// lookbackBars <= 0 -> keep all candles returned by Alpha Vantage
std::unique_ptr<IMarketDataFeed>
makeAlphaVantageFeed(const std::string &apiKey,
                     const std::string &symbol,
                     int lookbackBars);
