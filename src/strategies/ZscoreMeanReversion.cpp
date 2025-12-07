#include "Strategy_I.hpp"
#include "BacktestEngine.hpp"
#include <deque>
#include <cmath>
#include <iostream>
#include <memory>

/*
 Z-SCORE MEAN REVERSION STRATEGY
 --------------------------------
 Uses a rolling mean + standard deviation to measure how far the current
 price deviates from its average. The deviation is normalized as:

      z = (price - mean) / stddev

 Signals:
   • Buy  when z <= entry_zscore   (price unusually low)
   • Sell when z >= exit_zscore    (price returns toward mean)

 Notes:
   • Works best in sideways / mean-reverting markets.
   • Performs poorly in strong trends.
   • lookback controls smoothing (short = faster, long = quieter).
   • Only trades long positions (no shorting).
*/

class ZScoreMeanReversion : public Strategy_I
{
public:
  ZScoreMeanReversion(std::string symbol,
                      int zWindow,
                      double zEntry,
                      double zExit)
    : symbol_(std::move(symbol)),
      zWindow_(zWindow),
      zEntry_(zEntry),
      zExit_(zExit)
  {
  }

  void onStart(BacktestEngine &) override
  {
    closes_.clear();
    std::cout << "ZScoreMeanReversionStrategy starting\n";
  }

  void onBar(std::size_t,
             const Candle &bar,
             BacktestEngine &engine) override
  {
    if(bar.symbol != symbol_)
    {
      return;
    }

    closes_.push_back(bar.close);

    while(closes_.size() > static_cast<std::size_t>(zWindow_))
    {
      closes_.pop_front();
    }

    if(closes_.size() < static_cast<std::size_t>(zWindow_))
    {
      return;
    }

    double z = computeZScore();
    const Position *pos = engine.portfolio().getPosition(symbol_);
    int qty = (pos != nullptr) ? pos->quantity : 0;

    // BUY SIGNAL
    if(qty == 0 && z < zEntry_)
    {
      Order buy;
      buy.symbol = symbol_;
      buy.side = OrderSide::Buy;
      buy.quantity = 100;
      engine.placeOrder(buy);
    }
    // EXIT SIGNAL
    else if(qty > 0 && z > zExit_)
    {
      Order sell;
      sell.symbol = symbol_;
      sell.side = OrderSide::Sell;
      sell.quantity = qty;
      engine.placeOrder(sell);
    }
  }

  void onEnd(BacktestEngine &engine) override
  {
    std::cout << "ZScoreMeanReversion finished. Final equity: "
              << engine.portfolio().getEquity() << "\n";
  }

private:
  double computeZScore() const
  {
    const std::size_t window = static_cast<std::size_t>(zWindow_);

    if(closes_.size() < window)
    {
      return 0.0;
    }

    const std::size_t n = closes_.size();
    const std::size_t start = n - window;

    double sum = 0.0;
    for(std::size_t i = start; i < n; ++i)
    {
      sum += closes_[i];
    }
    double mean = sum / static_cast<double>(window);

    double sq = 0.0;
    for(std::size_t i = start; i < n; ++i)
    {
      sq += (closes_[i] - mean) * (closes_[i] - mean);
    }
    double sd = std::sqrt(sq / static_cast<double>(window));

    if(sd == 0.0)
    {
      return 0.0;
    }

    return (closes_.back() - mean) / sd;
  }

  std::string symbol_;
  int zWindow_;
  double zEntry_;
  double zExit_;
  std::deque<double> closes_;
};

std::unique_ptr<Strategy_I>
makeZScoreMeanReversionStrategy(const std::string &symbol,
                                int zWindow,
                                double zEntry,
                                double zExit)
{
  return std::make_unique<ZScoreMeanReversion>(
    symbol,
    zWindow,
    zEntry,
    zExit);
}
