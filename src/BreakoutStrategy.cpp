#include "ITradingStrategy.h"
#include "BacktestEngine.h"
#include "Portfolio.h"

#include <vector>
#include <iostream>
#include <memory>
#include <utility>

class BreakoutStrategy : public ITradingStrategy
{
public:
  BreakoutStrategy(std::string symbol,
                   std::size_t lookbackWindow)
    : symbol_(std::move(symbol)),
      lookbackWindow_(lookbackWindow),
      trades_(0)
  {
  }

  void onStart(BacktestEngine & /*engine*/) override
  {
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
    highs_.push_back(bar.high);
    lows_.push_back(bar.low);
    closes_.push_back(bar.close);

    // Need at least lookbackWindow_ + 1 bars so that
    // we can use the PREVIOUS lookbackWindow_ bars as the range.
    if(!hasEnoughHistory())
    {
      return;
    }

    double close = bar.close;
    double rangeHigh = recentMaxHigh();
    double rangeLow  = recentMinLow();

    int posQty = engine.portfolio().getPositionQty(symbol_);

    // Classic breakout:
    //  - close > previous N-bar high and flat  -> enter long
    //  - close < previous N-bar low  and long  -> exit
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
    // Need enough bars to have a full previous-window range
    //   size >= lookbackWindow_ + 1
    if(highs_.size() >= lookbackWindow_ + 1)
    {
      return true;
    }
    return false;
  }

  // Previous N-bar high, excluding the current bar.
  double recentMaxHigh() const
  {
    std::size_t N = highs_.size();
    // Current bar is at index N-1.
    // Use [start, end] = [N-1-lookbackWindow_, N-2].
    std::size_t end   = N - 2;
    std::size_t start = end - lookbackWindow_ + 1;

    double maxH = highs_[start];
    for(std::size_t i = start + 1; i <= end; ++i)
    {
      if(highs_[i] > maxH)
      {
        maxH = highs_[i];
      }
    }
    return maxH;
  }

  // Previous N-bar low, excluding the current bar.
  double recentMinLow() const
  {
    std::size_t N = lows_.size();
    std::size_t end   = N - 2;
    std::size_t start = end - lookbackWindow_ + 1;

    double minL = lows_[start];
    for(std::size_t i = start + 1; i <= end; ++i)
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
  int                 trades_;
};

std::unique_ptr<ITradingStrategy>
makeBreakoutStrategy(const std::string &symbol,
                     std::size_t lookbackWindow)
{
  return std::make_unique<BreakoutStrategy>(symbol, lookbackWindow);
}
