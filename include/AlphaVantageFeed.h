#pragma once

#include "IMarketDataFeed.h"
#include <memory>
#include <string>

class AlphaVantageFeed : public IMarketDataFeed
{
public:
  AlphaVantageFeed(const std::string &apiKey,
                   const std::string &symbol);

  bool hasNext() const override;
  const Candle &next() override;
  std::size_t currentIndex() const override;

private:
  void loadDailySeries(const std::string &apiKey,
                       const std::string &symbol);

  std::vector<Candle> candles_;
  std::size_t index_{ 0 };
};

// Factory function (similar style to makeVectorFeed)
std::unique_ptr<IMarketDataFeed>
makeAlphaVantageFeed(const std::string &apiKey,
                     const std::string &symbol);
