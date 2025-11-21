#include "IMarketDataFeed.h"
#include <vector>
#include <memory>
#include <utility> // for std::move

// A simple in-memory implementation of IMarketDataFeed based on std::vector<Candle>.
class VectorMarketDataFeed : public IMarketDataFeed
{
public:
  explicit VectorMarketDataFeed(std::vector<Candle> candles)
    : candles_(std::move(candles)), index_(0) {}

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
    // If next() has never been called, we treat current index as 0.
    return index_ == 0 ? 0 : index_ - 1;
  }

private:
  std::vector<Candle> candles_;
  std::size_t index_;
};

// Factory function defined in the .cpp, declared in IMarketDataFeed.h
std::unique_ptr<IMarketDataFeed>
makeVectorFeed(std::vector<Candle> candles)
{
  return std::make_unique<VectorMarketDataFeed>(std::move(candles));
}
