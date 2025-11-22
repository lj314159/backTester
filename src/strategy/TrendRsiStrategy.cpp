#include "TradingStrategy_I.h"
#include "BacktestEngine.h"
#include "Portfolio.h"

#include <vector>
#include <iostream>
#include <memory>
#include <utility>
#include <cmath>

class TrendRsiStrategy : public TradingStrategy_I
{
public:
  TrendRsiStrategy(std::string symbol,
                   std::size_t rsiPeriod,
                   double overbought,
                   double oversold,
                   std::size_t trendWindow)
    : symbol_(std::move(symbol)),
      rsiPeriod_(rsiPeriod),
      overbought_(overbought),
      oversold_(oversold),
      trendWindow_(trendWindow),
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
    std::cout << "TrendRsiStrategy starting\n";
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

    double trendSma = computeSMA(trendWindow_);
    double rsi = computeRSI(rsiPeriod_);

    if(rsi < minRsi_)
    {
      minRsi_ = rsi;
    }
    if(rsi > maxRsi_)
    {
      maxRsi_ = rsi;
    }

    bool uptrend = bar.close > trendSma;

    int posQty = engine.portfolio().getPositionQty(symbol_);

    // Entry: only buy dips in an uptrend.
    if(uptrend && rsi <= oversold_ && posQty == 0)
    {
      enterLong(engine, 100);
      ++trades_;
    }
    // Exit: either trend breaks or RSI normalizes / overbought.
    else if(posQty > 0 && (!uptrend || rsi >= overbought_))
    {
      exitLong(engine, posQty);
      ++trades_;
    }
  }

  void onEnd(BacktestEngine &engine) override
  {
    std::cout << "TrendRsiStrategy finished. Final cash: "
              << engine.portfolio().getCash() << "\n";
    std::cout << "Trend/RSI trades taken: " << trades_ << "\n";
    std::cout << "RSI min: " << minRsi_
              << "  max: " << maxRsi_ << "\n";
  }

private:
  bool hasEnoughHistory() const
  {
    std::size_t need = trendWindow_;
    std::size_t needRsi = rsiPeriod_ + 1;
    if(needRsi > need)
    {
      need = needRsi;
    }

    if(closes_.size() >= need)
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

  double computeRSI(std::size_t period) const
  {
    double gains = 0.0;
    double losses = 0.0;

    std::size_t end = closes_.size() - 1;
    std::size_t start = end - period + 1;

    for(std::size_t i = start; i <= end; ++i)
    {
      double diff = closes_[i] - closes_[i - 1];
      if(diff > 0.0)
      {
        gains += diff;
      }
      else if(diff < 0.0)
      {
        losses -= diff;
      }
    }

    double avgGain = gains / static_cast<double>(period);
    double avgLoss = losses / static_cast<double>(period);

    if(avgLoss == 0.0)
    {
      return 100.0;
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
  std::size_t rsiPeriod_;
  double overbought_;
  double oversold_;
  std::size_t trendWindow_;
  std::vector<double> closes_;

  int trades_;
  double minRsi_;
  double maxRsi_;
};

std::unique_ptr<TradingStrategy_I>
makeTrendRsiStrategy(const std::string &symbol,
                     std::size_t rsiPeriod,
                     double overbought,
                     double oversold,
                     std::size_t trendWindow)
{
  return std::make_unique<TrendRsiStrategy>(symbol,
                                            rsiPeriod,
                                            overbought,
                                            oversold,
                                            trendWindow);
}
