#include "TradingStrategy_I.h"
#include "BacktestEngine.h"
#include "Portfolio.h"

#include <vector>
#include <iostream>
#include <memory>
#include <utility>

class SimpleSMAStrategy : public ITradingStrategy
{
public:
  SimpleSMAStrategy(std::string symbol, std::size_t window)
    : symbol_(std::move(symbol)),
      window_(window)
  {
  }

  void onStart(BacktestEngine & /*engine*/) override
  {
    closes_.clear();
    std::cout << "SimpleSMAStrategy starting\n";
  }

  void onBar(std::size_t /*index*/,
             const Candle &bar,
             BacktestEngine &engine) override
  {
    closes_.push_back(bar.close);

    if(closes_.size() < window_)
    {
      return;
    }

    double sma = computeSMA();
    int posQty = engine.portfolio().getPositionQty(symbol_);

    // 4) Trading logic is now very readable.
    if(bar.close > sma && posQty == 0)
    {
      enterLong(engine, 100);
    }
    else if(bar.close < sma && posQty > 0)
    {
      exitLong(engine, posQty);
    }
  }

  void onEnd(BacktestEngine &engine) override
  {
    std::cout << "SimpleSMAStrategy finished. Final cash: "
              << engine.portfolio().getCash() << "\n";
  }

private:
  // --- Helper methods that hide the plumbing ---

  void updateHistory(double close)
  {
    closes_.push_back(close);
  }

  bool hasEnoughHistory() const
  {
    if(closes_.size() >= window_)
    {
      return true;
    }
    return false;
  }

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

  int currentPositionQty(BacktestEngine &engine) const
  {
    return engine.portfolio().getPositionQty(symbol_);
  }

  void enterLong(BacktestEngine &engine, int quantity)
  {
    Order buy;
    buy.symbol = symbol_;
    buy.side = OrderSide::Buy;
    buy.quantity = quantity;
    engine.placeOrder(buy);
  }

  void exitLong(BacktestEngine &engine, int quantity)
  {
    Order sell;
    sell.symbol = symbol_;
    sell.side = OrderSide::Sell;
    sell.quantity = quantity;
    engine.placeOrder(sell);
  }

  // --- Data members ---

  std::string symbol_;
  std::size_t window_;
  std::vector<double> closes_;
};

// Factory function used by main.cpp
std::unique_ptr<ITradingStrategy>
makeSimpleSMAStrategy(const std::string &symbol, std::size_t window)
{
  return std::make_unique<SimpleSMAStrategy>(symbol, window);
}
