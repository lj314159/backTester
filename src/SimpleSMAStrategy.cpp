#include "ITradingStrategy.h"
#include "BacktestEngine.h"
#include "Portfolio.h"
#include <vector>
#include <numeric>
#include <iostream>
#include <cmath>

class SimpleSMAStrategy : public ITradingStrategy
{
public:
  SimpleSMAStrategy(std::string symbol, std::size_t window)
    : symbol_(std::move(symbol)), window_(window) {}

  void onStart(BacktestEngine & /*engine*/) override
  {
    closes_.clear();
    std::cout << "SimpleSMAStrategy starting\n";
  }

  void onBar(std::size_t index,
             const Candle &bar,
             BacktestEngine &engine) override
  {
    // We assume the feed is for a single symbol (symbol_).
    // So we don't check bar.symbol; Candle does not have that field.

    closes_.push_back(bar.close);
    if(closes_.size() < window_)
    {
      return; // not enough data to compute SMA yet
    }

    double sma = computeSMA();
    int posQty = engine.portfolio().getPositionQty(symbol_);

    // Very naive trading logic:
    // If close > SMA and no position -> buy 100
    if(bar.close > sma && posQty == 0)
    {
      Order buy;
      buy.symbol = symbol_;
      buy.side = OrderSide::Buy;
      buy.quantity = 100;
      engine.placeOrder(buy);
    }

    // If close < SMA and we have a position -> sell all
    if(bar.close < sma && posQty > 0)
    {
      Order sell;
      sell.symbol = symbol_;
      sell.side = OrderSide::Sell;
      sell.quantity = posQty;
      engine.placeOrder(sell);
    }
  }

  void onEnd(BacktestEngine &engine) override
  {
    std::cout << "SimpleSMAStrategy finished. "
              << "Final cash: " << engine.portfolio().getCash()
              << "\n";
  }

private:
  double computeSMA() const
  {
    double sum = 0.0;
    std::size_t start = closes_.size() - window_;
    for(std::size_t i = start; i < closes_.size(); ++i)
    {
      sum += closes_[i];
    }
    return sum / static_cast<double>(window_);
  }

  std::string symbol_;
  std::size_t window_;
  std::vector<double> closes_;
};

// Factory function (declared in ITradingStrategy.h)
std::unique_ptr<ITradingStrategy>
makeSimpleSMAStrategy(const std::string &symbol, std::size_t window)
{
  return std::make_unique<SimpleSMAStrategy>(symbol, window);
}
