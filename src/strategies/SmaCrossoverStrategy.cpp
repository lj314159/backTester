#include "Strategy_I.hpp"
#include "BacktestEngine.hpp"
#include <deque>
#include <iostream>
#include <memory>

class SmaCrossoverStrategy : public Strategy_I
{
public:
  SmaCrossoverStrategy(std::string symbol,
                       int shortPeriod,
                       int longPeriod)
    : symbol_(std::move(symbol)),
      shortPeriod_(shortPeriod),
      longPeriod_(longPeriod) {}

  void onStart(BacktestEngine &engine) override
  {
    (void)engine;
    closes_.clear();
    std::cout << "SmaCrossoverStrategy starting\n";
  }

  void onBar(std::size_t /*index*/,
             const Candle &bar,
             BacktestEngine &engine) override
  {
    if(bar.symbol != symbol_)
    {
      return;
    }

    closes_.push_back(bar.close);
    while((int)closes_.size() > longPeriod_)
    {
      closes_.pop_front();
    }
    if((int)closes_.size() < longPeriod_)
    {
      return;
    }

    double shortSma = computeSMA(shortPeriod_);
    double longSma = computeSMA(longPeriod_);

    const Position *pos = engine.portfolio().getPosition(symbol_);
    int qty = pos ? pos->quantity : 0;

    if(qty == 0 && shortSma > longSma)
    {
      Order buy;
      buy.symbol = symbol_;
      buy.side = OrderSide::Buy;
      buy.quantity = 100;
      engine.placeOrder(buy);
    }
    else if(qty > 0 && shortSma < longSma && pos)
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
    std::cout << "SmaCrossoverStrategy finished. Final equity: "
              << engine.portfolio().getEquity() << "\n";
  }

private:
  double computeSMA(int period) const
  {
    if((int)closes_.size() < period)
    {
      return 0.0;
    }
    double sum = 0.0;
    auto it = closes_.end();
    for(int i = 0; i < period; ++i)
    {
      --it;
      sum += *it;
    }
    return sum / period;
  }

  std::string symbol_;
  int shortPeriod_;
  int longPeriod_;
  std::deque<double> closes_;
};

std::unique_ptr<Strategy_I>
makeSmaCrossoverStrategy(const std::string &symbol,
                         int shortPeriod,
                         int longPeriod)
{
  return std::make_unique<SmaCrossoverStrategy>(
    symbol, shortPeriod, longPeriod);
}
