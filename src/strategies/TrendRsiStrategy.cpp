#include "Strategy_I.hpp"
#include "BacktestEngine.hpp"
#include <deque>
#include <cmath>
#include <iostream>
#include <memory>

class TrendRsiStrategy : public Strategy_I
{
public:
  TrendRsiStrategy(std::string symbol,
                   int period,
                   double overbought,
                   double oversold,
                   int trendWindow)
    : symbol_(std::move(symbol)),
      period_(period),
      overbought_(overbought),
      oversold_(oversold),
      trendWindow_(trendWindow) {}

  void onStart(BacktestEngine &engine) override
  {
    (void)engine;
    closes_.clear();
    std::cout << "TrendRsiStrategy starting\n";
  }

  void onBar(std::size_t /*index*/,
             const Candle &bar,
             BacktestEngine &engine) override
  {
    if (bar.symbol != symbol_) {
      return;
    }

    closes_.push_back(bar.close);

    // maxNeeded is how many closes we need to keep in the window
    const int maxNeededInt = std::max(period_ + 1, trendWindow_);
    const std::size_t maxNeeded =
        static_cast<std::size_t>(maxNeededInt);

    while (closes_.size() > maxNeeded) {
      closes_.pop_front();
    }
    if (closes_.size() < maxNeeded) {
      return;
    }

    double rsi       = computeRSI();
    double trendSma  = computeTrendSMA();
    double price     = bar.close;

    const Position *pos = engine.portfolio().getPosition(symbol_);
    int qty = pos ? pos->quantity : 0;

    if (qty == 0 && rsi < oversold_ && price > trendSma) {
      Order buy;
      buy.symbol   = symbol_;
      buy.side     = OrderSide::Buy;
      buy.quantity = 100;
      engine.placeOrder(buy);
    } else if (qty > 0 && (rsi > overbought_ || price < trendSma)) {
      Order sell;
      sell.symbol   = symbol_;
      sell.side     = OrderSide::Sell;
      sell.quantity = qty;
      engine.placeOrder(sell);
    }
  }

  void onEnd(BacktestEngine &engine) override
  {
    std::cout << "TrendRsiStrategy finished. Final equity: "
              << engine.portfolio().getEquity() << "\n";
  }

private:
  double computeRSI() const
  {
    const std::size_t needed = static_cast<std::size_t>(period_ + 1);

    if (closes_.size() < needed) {
      return 50.0;
    }

    double gain = 0.0;
    double loss = 0.0;

    const std::size_t n     = closes_.size();
    const std::size_t start = n - static_cast<std::size_t>(period_);

    for (std::size_t i = start; i < n; ++i) {
      double diff = closes_[i] - closes_[i - 1];
      if (diff > 0) {
        gain += diff;
      } else {
        loss -= diff;
      }
    }

    double avgG = gain / static_cast<double>(period_);
    double avgL = loss / static_cast<double>(period_);
    if (avgL == 0.0) {
      return avgG == 0.0 ? 50.0 : 100.0;
    }
    double rs = avgG / avgL;
    return 100.0 - 100.0 / (1.0 + rs);
  }

  double computeTrendSMA() const
  {
    const std::size_t window = static_cast<std::size_t>(trendWindow_);

    if (closes_.size() < window) {
      return closes_.back();
    }

    const std::size_t n     = closes_.size();
    const std::size_t start = n - window;

    double sum = 0.0;
    for (std::size_t i = start; i < n; ++i) {
      sum += closes_[i];
    }

    return sum / static_cast<double>(trendWindow_);
  }

  std::string symbol_;
  int period_;
  double overbought_;
  double oversold_;
  int trendWindow_;
  std::deque<double> closes_;
};

std::unique_ptr<Strategy_I>
makeTrendRsiStrategy(const std::string &symbol,
                     int period,
                     double overbought,
                     double oversold,
                     int trendWindow)
{
  return std::make_unique<TrendRsiStrategy>(
    symbol, period, overbought, oversold, trendWindow);
}
