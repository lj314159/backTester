#include "TradingStrategy_I.h"
#include "BacktestEngine.h"
#include "Portfolio.h"

#include <vector>
#include <iostream>
#include <memory>
#include <utility>
#include <cmath>

class RsiReversionStrategy : public TradingStrategy_I
{
public:
  RsiReversionStrategy(std::string symbol,
                       std::size_t period,
                       double overbought,
                       double oversold)
    : symbol_(std::move(symbol)),
      period_(period),
      overbought_(overbought),
      oversold_(oversold),
      trades_(0),
      minRsi_(100.0),
      maxRsi_(0.0)
  {
  }

  void onStart(BacktestEngine & /*engine*/) override
  {
    closes_.clear();
    trades_ = 0;
    minRsi_ = 100.0;
    maxRsi_ = 0.0;
    std::cout << "RsiReversionStrategy starting\n";
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

    double rsi = computeRSI();
    if(rsi < minRsi_)
    {
      minRsi_ = rsi;
    }
    if(rsi > maxRsi_)
    {
      maxRsi_ = rsi;
    }

    int posQty = engine.portfolio().getPositionQty(symbol_);

    // Mean reversion logic:
    //  - if oversold and flat -> buy
    //  - if overbought and long -> sell
    if(rsi < oversold_ && posQty == 0)
    {
      enterLong(engine, 100);
      ++trades_;
    }
    else if(rsi > overbought_ && posQty > 0)
    {
      exitLong(engine, posQty);
      ++trades_;
    }
  }

  void onEnd(BacktestEngine &engine) override
  {
    std::cout << "RsiReversionStrategy finished. Final cash: "
              << engine.portfolio().getCash() << "\n";
    std::cout << "RSI trades taken: " << trades_ << "\n";
    std::cout << "RSI min: " << minRsi_
              << "  max: " << maxRsi_ << "\n";
  }

private:
  bool hasEnoughHistory() const
  {
    if(closes_.size() >= period_ + 1)
    {
      return true;
    }
    return false;
  }

  double computeRSI() const
  {
    double gains = 0.0;
    double losses = 0.0;

    std::size_t end = closes_.size() - 1;
    std::size_t start = end - period_ + 1;

    for(std::size_t i = start; i <= end; ++i)
    {
      double diff = closes_[i] - closes_[i - 1];
      if(diff > 0.0)
      {
        gains += diff;
      }
      else if(diff < 0.0)
      {
        losses -= diff; // diff is negative, subtract to make it positive
      }
    }

    double avgGain = gains / static_cast<double>(period_);
    double avgLoss = losses / static_cast<double>(period_);

    if(avgLoss == 0.0)
    {
      return 100.0; // no losses => RSI = 100
    }

    double rs = avgGain / avgLoss;
    double rsi = 100.0 - 100.0 / (1.0 + rs);
    return rsi;
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
  std::size_t period_;
  double overbought_;
  double oversold_;
  std::vector<double> closes_;

  int trades_;
  double minRsi_;
  double maxRsi_;
};

std::unique_ptr<TradingStrategy_I>
makeRsiReversionStrategy(const std::string &symbol,
                         std::size_t period,
                         double overbought,
                         double oversold)
{
  return std::make_unique<RsiReversionStrategy>(symbol,
                                                period,
                                                overbought,
                                                oversold);
}
