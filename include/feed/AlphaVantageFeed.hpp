#pragma once

#include <memory>
#include <string>
#include <vector>

#include "DataFeed_I.hpp"
#include "TradingTypes.hpp"

class AlphaVantageFeed : public DataFeed_I
{
public:
  explicit AlphaVantageFeed(std::vector<Candle> candles);

  bool hasNext() const override;
  const Candle &next() override;

  std::size_t currentIndex() const;

private:
  std::vector<Candle> candles_;
  std::size_t index_;
};

std::unique_ptr<DataFeed_I>
makeAlphaVantageFeed(const std::string &apiKey,
                     const std::string &symbol,
                     int lookbackBars);
