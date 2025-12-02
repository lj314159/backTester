# Backtesting Engine (C++)

A lightweight, dependency-free C++ backtesting engine designed for speed, simplicity, and full control.  
The engine uses only standard C++ headers and requires an Alpha Vantage API key for downloading historical data.

## Features

- Zero external dependencies (pure C++17/C++20)
- Event-driven backtesting architecture
- Required Alpha Vantage data downloader
- Simple Strategy interface
- Portfolio, PnL, and order simulation
- Basic execution models (market or limit)
- Configurable backtest parameters
- Works on Linux

## Requirements

- GCC
- Alpha Vantage API key (required)  
- CMake (for building the project)  
- No external libraries

## Getting an Alpha Vantage API Key

1. Visit the Alpha Vantage website  
2. Create a free account  
3. Copy your API key  
4. Set it as an environment variable:

    export ALPHA_VANTAGE_KEY=your_key_here

The engine uses this environment variable to authenticate all data requests.

## Building the Project (CMake)

Build steps:

    git clone <repo-url>
    cd <project-directory>
    mkdir build
    cd build
    cmake ..
    make

This creates the executable:

    backtest_engine

## Running a Backtest

A backtest is run using a JSON configuration file:

    ./backtest_engine ../config.json

Example config:

{
  "asset": "AAPL",
  "initial_cash": 100000.0,
  "data": {
    "provider": "alpha_vantage",
    "interval": "daily",
    "lookback_bars": -1
  },
  "strategy": {
    "name": "trend_rsi",
    "params": {
      "period": 14,
      "overbought": 60.0,
      "oversold": 40.0,
      "trend_window": 50
    }
  }
}

## Example Strategy: TrendRsiStrategy

```cpp
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
    if(bar.symbol != symbol_)
    {
      return;
    }

    closes_.push_back(bar.close);

    // maxNeeded is how many closes we need to keep in the window
    const int maxNeededInt = std::max(period_ + 1, trendWindow_);
    const std::size_t maxNeeded = static_cast<std::size_t>(maxNeededInt);

    while(closes_.size() > maxNeeded)
    {
      closes_.pop_front();
    }
    if(closes_.size() < maxNeeded)
    {
      return;
    }

    double rsi = computeRSI();
    double trendSma = computeTrendSMA();
    double price = bar.close;

    const Position *pos = engine.portfolio().getPosition(symbol_);
    int qty = pos ? pos->quantity : 0;

    if(qty == 0 && rsi < oversold_ && price > trendSma)
    {
      Order buy;
      buy.symbol = symbol_;
      buy.side = OrderSide::Buy;
      buy.quantity = 100;
      engine.placeOrder(buy);
    }
    else if(qty > 0 && (rsi > overbought_ || price < trendSma))
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
    std::cout << "TrendRsiStrategy finished. Final equity: "
              << engine.portfolio().getEquity() << "\n";
  }

private:
  double computeRSI() const
  {
    const std::size_t needed = static_cast<std::size_t>(period_ + 1);

    if(closes_.size() < needed)
    {
      return 50.0;
    }

    double gain = 0.0;
    double loss = 0.0;

    const std::size_t n = closes_.size();
    const std::size_t start = n - static_cast<std::size_t>(period_);

    for(std::size_t i = start; i < n; ++i)
    {
      double diff = closes_[i] - closes_[i - 1];
      if(diff > 0)
      {
        gain += diff;
      }
      else
      {
        loss -= diff;
      }
    }

    double avgG = gain / static_cast<double>(period_);
    double avgL = loss / static_cast<double>(period_);
    if(avgL == 0.0)
    {
      return avgG == 0.0 ? 50.0 : 100.0;
    }
    double rs = avgG / avgL;
    return 100.0 - 100.0 / (1.0 + rs);
  }

  double computeTrendSMA() const
  {
    const std::size_t window = static_cast<std::size_t>(trendWindow_);

    if(closes_.size() < window)
    {
      return closes_.back();
    }

    const std::size_t n = closes_.size();
    const std::size_t start = n - window;

    double sum = 0.0;
    for(std::size_t i = start; i < n; ++i)
    {
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

```

## Example Backtest Output

Below is an example run of the engine using:

```
./backtest_engine ../config.json
```

**Output:**

```text
build $ ./backtest_engine ../config.json

Fetching data from Alpha Vantage for AAPL (TIME_SERIES_DAILY, lookback_bars=-1)...
Fetched 100 candles. Date range: 2025-07-11 -> 2025-12-01
Running backtest...
  Strategy: trend_rsi
  Symbol:   AAPL
  Cash:     100000
TrendRsiStrategy starting
TrendRsiStrategy finished. Final equity: 102354

===== Backtest Results =====
Initial equity: 100000
Final equity:   102354
Total return:   2.354%
CAGR:           6.10146%
Sharpe:         0.401873
Max drawdown:   24.527%

build $
```

## License (MIT)

    MIT License  
    Copyright (c) 2025

