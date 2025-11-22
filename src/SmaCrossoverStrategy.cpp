#include "ITradingStrategy.h"
#include "BacktestEngine.h"
#include "Portfolio.h"

#include <vector>
#include <iostream>
#include <memory>
#include <utility>

class SmaCrossoverStrategy : public ITradingStrategy
{
public:
  SmaCrossoverStrategy(std::string symbol,
                       std::size_t fastWindow,
                       std::size_t slowWindow)
    : symbol_(std::move(symbol)),
      fastWindow_(fastWindow),
      slowWindow_(slowWindow),
      trades_(0)
  {
  }

  void onStart(BacktestEngine & /*engine*/) override
  {
    closes_.clear();
    trades_ = 0;
    std::cout << "SmaCrossoverStrategy starting\n";
  }

  void onBar(std::size_t /*index*/,
             const Candle &bar,
             BacktestEngine &engine) override
  {
    closes_.push_back(bar.close);

    // Need at least slowWindow_ points to compute both SMAs
    if(!hasEnoughHistory())
    {
      return;
    }

    double fast = computeSMA(fastWindow_);
    double slow = computeSMA(slowWindow_);

    int posQty = engine.portfolio().getPositionQty(symbol_);

    // Simple trend-follow logic:
    //  - if fast > slow and we are flat -> go long
    //  - if fast < slow and we are long -> exit
    if(fast > slow && posQty == 0)
    {
      enterLong(engine, 100);
      ++trades_;
    }
    else if(fast < slow && posQty > 0)
    {
      exitLong(engine, posQty);
      ++trades_;
    }
  }

  void onEnd(BacktestEngine &engine) override
  {
    std::cout << "SmaCrossoverStrategy finished. Final cash: "
              << engine.portfolio().getCash() << "\n";
    std::cout << "SMA crossover trades taken: " << trades_ << "\n";
  }

private:
  bool hasEnoughHistory() const
  {
    if(closes_.size() >= slowWindow_)
    {
      return true;
    }
    return false;
  }

  double computeSMA(std::size_t window) const
  {
    double sum = 0.0;
    std::size_t end = closes_.size();
    std::size_t start = end - window;
    for(std::size_t i = start; i < end; ++i)
    {
      sum += closes_[i];
    }
    return sum / static_cast<double>(window);
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
  std::size_t         fastWindow_;
  std::size_t         slowWindow_;
  std::vector<double> closes_;
  int                 trades_;
};

std::unique_ptr<ITradingStrategy>
makeSmaCrossoverStrategy(const std::string &symbol,
                         std::size_t fastWindow,
                         std::size_t slowWindow)
{
  return std::make_unique<SmaCrossoverStrategy>(symbol,
                                                fastWindow,
                                                slowWindow);
}
