#include "MarketDataFeed_I.h"
#include <vector>
#include <memory>
#include <utility>

class VectorMarketDataFeed : public IMarketDataFeed
{
public:
  explicit VectorMarketDataFeed(std::vector<Candle> candles)
    : candles_(std::move(candles)), index_(0)
  {
  }

  bool hasNext() const override
  {
    return index_ < candles_.size();
  }

  const Candle &next() override
  {
    return candles_.at(index_++);
  }

  std::size_t currentIndex() const override
  {
    if(index_ == 0)
    {
      return 0;
    }
    return index_ - 1;
  }

private:
  std::vector<Candle> candles_;
  std::size_t index_;
};

std::unique_ptr<IMarketDataFeed>
makeVectorFeed(std::vector<Candle> candles)
{
  return std::make_unique<VectorMarketDataFeed>(std::move(candles));
}
