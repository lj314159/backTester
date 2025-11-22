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
      slowWindow_(slowWindow)
  {
  }

  void onStart(BacktestEngine & /*engine*/) override
  {
    closes_.clear();
    std::cout << "SmaCrossoverStrategy starting\n";
  }

  void onBar(std::size_t /*index*/,
             const Candle &bar,
             BacktestEngine &engine) override
  {
    closes_.push_back(bar.close);

    if(!hasEnoughHistory())
    {
      return;
    }

    double fastNow = computeSMA(fastWindow_);
    double slowNow = computeSMA(slowWindow_);

    double fastPrev = computeSmaPrev(fastWindow_);
    double slowPrev = computeSmaPrev(slowWindow_);

    int posQty = engine.portfolio().getPositionQty(symbol_);

    bool goldenCross = (fastPrev <= slowPrev && fastNow > slowNow);
    bool deathCross  = (fastPrev >= slowPrev && fastNow < slowNow);

    if(goldenCross && posQty == 0)
    {
      enterLong(engine, 100);
    }
    else if(deathCross && posQty > 0)
    {
      exitLong(engine, posQty);
    }
  }

  void onEnd(BacktestEngine &engine) override
  {
    std::cout << "SmaCrossoverStrategy finished. Final cash: "
              << engine.portfolio().getCash() << "\n";
  }

private:
  bool hasEnoughHistory() const
  {
    if(closes_.size() >= slowWindow_ + 1)
    {
      return true;
    }
    return false;
  }

  double computeSMA(std::size_t window) const
  {
    double sum = 0.0;
    std::size_t start = closes_.size() - window;
    for(std::size_t i = start; i < closes_.size(); ++i)
    {
      sum += closes_[i];
    }
    return sum / static_cast<double>(window);
  }

  double computeSmaPrev(std::size_t window) const
  {
    double sum = 0.0;
    std::size_t end   = closes_.size() - 1;
    std::size_t start = end - window + 1;
    for(std::size_t i = start; i <= end; ++i)
    {
      sum += closes_[i];
    }
    return sum / static_cast<double>(window);
  }

  void enterLong(BacktestEngine &engine, int quantity)
  {
    Order buy;
    buy.symbol   = symbol_;
    buy.side     = OrderSide::Buy;
    buy.quantity = quantity;
    engine.placeOrder(buy);
  }

  void exitLong(BacktestEngine &engine, int quantity)
  {
    Order sell;
    sell.symbol   = symbol_;
    sell.side     = OrderSide::Sell;
    sell.quantity = quantity;
    engine.placeOrder(sell);
  }

  std::string        symbol_;
  std::size_t        fastWindow_;
  std::size_t        slowWindow_;
  std::vector<double> closes_;
};

std::unique_ptr<ITradingStrategy>
makeSmaCrossoverStrategy(const std::string &symbol,
                         std::size_t fastWindow,
                         std::size_t slowWindow)
{
  return std::make_unique<SmaCrossoverStrategy>(symbol, fastWindow, slowWindow);
}
