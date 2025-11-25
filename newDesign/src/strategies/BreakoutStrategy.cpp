#include "Strategy_I.hpp"
#include "BacktestEngine.hpp"
#include <vector>
#include <iostream>
#include <memory>
#include <utility>

class BreakoutStrategy : public Strategy_I
{
public:
  BreakoutStrategy(std::string symbol,
                   std::size_t lookbackWindow)
    : symbol_(std::move(symbol)),
      lookbackWindow_(lookbackWindow),
      trades_(0)
  {
  }

  void onStart(BacktestEngine &engine) override
  {
    (void)engine;
    highs_.clear();
    lows_.clear();
    closes_.clear();
    trades_ = 0;
    std::cout << "BreakoutStrategy starting\n";
  }

  void onBar(std::size_t /*index*/,
             const Candle &bar,
             BacktestEngine &engine) override
  {
    if(bar.symbol != symbol_)
      return;

    highs_.push_back(bar.high);
    lows_.push_back(bar.low);
    closes_.push_back(bar.close);

    if(!hasEnoughHistory())
      return;

    double close = bar.close;
    double rangeHigh = recentMaxHigh();
    double rangeLow = recentMinLow();

    const Position *pos = engine.portfolio().getPosition(symbol_);
    int posQty = pos ? pos->quantity : 0;

    if(close > rangeHigh && posQty == 0)
    {
      enterLong(engine, 100);
      ++trades_;
    }
    else if(close < rangeLow && posQty > 0)
    {
      exitLong(engine, posQty);
      ++trades_;
    }
  }

  void onEnd(BacktestEngine &engine) override
  {
    std::cout << "BreakoutStrategy finished. Final cash: "
              << engine.portfolio().getCash() << "\n";
    std::cout << "Breakout trades taken: " << trades_ << "\n";
  }

private:
  bool hasEnoughHistory() const
  {
    return highs_.size() >= lookbackWindow_ + 1;
  }

  double recentMaxHigh() const
  {
    std::size_t N = highs_.size();
    std::size_t end = N - 2;
    std::size_t start = end - lookbackWindow_ + 1;

    double maxH = highs_[start];
    for(std::size_t i = start + 1; i <= end; ++i)
    {
      if(highs_[i] > maxH)
        maxH = highs_[i];
    }
    return maxH;
  }

  double recentMinLow() const
  {
    std::size_t N = lows_.size();
    std::size_t end = N - 2;
    std::size_t start = end - lookbackWindow_ + 1;

    double minL = lows_[start];
    for(std::size_t i = start + 1; i <= end; ++i)
    {
      if(lows_[i] < minL)
        minL = lows_[i];
    }
    return minL;
  }

  void enterLong(BacktestEngine &engine, int quantity) const
  {
    Order buy;
    buy.symbol = symbol_;
    buy.side = OrderSide::Buy;
    buy.quantity = quantity;
    engine.placeOrder(buy);
  }

  void exitLong(BacktestEngine &engine, int quantity) const
  {
    Order sell;
    sell.symbol = symbol_;
    sell.side = OrderSide::Sell;
    sell.quantity = quantity;
    engine.placeOrder(sell);
  }

  std::string symbol_;
  std::size_t lookbackWindow_;
  std::vector<double> highs_;
  std::vector<double> lows_;
  std::vector<double> closes_;
  int trades_;
};

std::unique_ptr<Strategy_I>
makeBreakoutStrategy(const std::string &symbol,
                     std::size_t lookbackWindow)
{
  return std::make_unique<BreakoutStrategy>(symbol, lookbackWindow);
}
