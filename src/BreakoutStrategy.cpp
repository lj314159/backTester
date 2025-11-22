#include "ITradingStrategy.h"
#include "BacktestEngine.h"
#include "Portfolio.h"

#include <vector>
#include <iostream>
#include <memory>
#include <utility>
#include <algorithm>

class BreakoutStrategy : public ITradingStrategy
{
public:
  BreakoutStrategy(std::string symbol,
                   std::size_t lookbackWindow)
    : symbol_(std::move(symbol)),
      lookbackWindow_(lookbackWindow)
  {
  }

  void onStart(BacktestEngine & /*engine*/) override
  {
    highs_.clear();
    lows_.clear();
    closes_.clear();
    std::cout << "BreakoutStrategy starting\n";
  }

  void onBar(std::size_t /*index*/,
             const Candle &bar,
             BacktestEngine &engine) override
  {
    highs_.push_back(bar.high);
    lows_.push_back(bar.low);
    closes_.push_back(bar.close);

    if(!hasEnoughHistory())
    {
      return;
    }

    double rangeHigh = recentMaxHigh();
    double rangeLow  = recentMinLow();
    double close     = bar.close;

    int posQty = engine.portfolio().getPositionQty(symbol_);

    if(close > rangeHigh && posQty == 0)
    {
      enterLong(engine, 100);
    }
    else if(close < rangeLow && posQty > 0)
    {
      exitLong(engine, posQty);
    }
  }

  void onEnd(BacktestEngine &engine) override
  {
    std::cout << "BreakoutStrategy finished. Final cash: "
              << engine.portfolio().getCash() << "\n";
  }

private:
  bool hasEnoughHistory() const
  {
    if(highs_.size() >= lookbackWindow_)
    {
      return true;
    }
    return false;
  }

  double recentMaxHigh() const
  {
    std::size_t end   = highs_.size();
    std::size_t start = end - lookbackWindow_;

    double maxH = highs_[start];
    for(std::size_t i = start + 1; i < end; ++i)
    {
      if(highs_[i] > maxH)
      {
        maxH = highs_[i];
      }
    }
    return maxH;
  }

  double recentMinLow() const
  {
    std::size_t end   = lows_.size();
    std::size_t start = end - lookbackWindow_;

    double minL = lows_[start];
    for(std::size_t i = start + 1; i < end; ++i)
    {
      if(lows_[i] < minL)
      {
        minL = lows_[i];
      }
    }
    return minL;
  }

  void enterLong(BacktestEngine &engine, int quantity) const
  {
    Order buy;
    buy.symbol   = symbol_;
    buy.side     = OrderSide::Buy;
    buy.quantity = quantity;
    engine.placeOrder(buy);
  }

  void exitLong(BacktestEngine &engine, int quantity) const
  {
    Order sell;
    sell.symbol   = symbol_;
    sell.side     = OrderSide::Sell;
    sell.quantity = quantity;
    engine.placeOrder(sell);
  }

  std::string         symbol_;
  std::size_t         lookbackWindow_;
  std::vector<double> highs_;
  std::vector<double> lows_;
  std::vector<double> closes_;
};

std::unique_ptr<ITradingStrategy>
makeBreakoutStrategy(const std::string &symbol,
                     std::size_t lookbackWindow)
{
  return std::make_unique<BreakoutStrategy>(symbol, lookbackWindow);
}
