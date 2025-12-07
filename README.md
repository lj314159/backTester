# Backtesting Engine (C++)

A lightweight, dependency-free C++ backtesting engine designed for speed, simplicity, and full control.  
The engine uses only standard C++ headers and requires an Alpha Vantage API key for downloading historical data.

## Features

- Minimal external dependencies (only libcurl)
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
- libcurl (for HTTP calls to Alpha Vantage)

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

## Example of a run configuration

A backtest is run using a JSON configuration file:

    ./backtest_engine ../configs/config.json

Example config.json:

```text
{
  "asset": "SPY",
  "initial_cash": 100000.0,
  "data": {
    "provider": "alpha_vantage",
    "interval": "daily",
    "lookback_bars": -1
  },
  "strategy": {
    "name": "mean_reversion_zscore",
    "params": {
      "lookback": 15,
      "entry_zscore": -1.5,
      "exit_zscore": -0.25,
      "ma_type": "ema"
    }
  }
}
```

## Example Strategy: Z-Score Mean Reversion

```cpp
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
```

## Example Backtest Output

Below is an example run of the engine using:

```
./backtest_engine ../configs/meanReversion.json
```

**Output:**

```text
build $ ./backtest_engine ../configs/meanReversion.json

Fetching data from Alpha Vantage for SPY (TIME_SERIES_DAILY, lookback_bars=-1)...
Fetched 100 candles. Date range: 2025-07-17 -> 2025-12-05
Running backtest...
  Strategy: mean_reversion_zscore
  Symbol:   SPY
  Cash:     100000
ZScoreMeanReversionStrategy starting
ZScoreMeanReversion finished. Final equity: 101521

===== Backtest Results =====
Initial equity: 100000
Final equity:   101521
Total return:   1.521%
CAGR:           3.91727%
Sharpe:         1.40573
Max drawdown:   67.0661%

build $
```

## License (MIT)

    MIT License  
    Copyright (c) 2025

