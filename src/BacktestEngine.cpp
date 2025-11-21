#include "BacktestEngine.h"
#include "ITradingStrategy.h"
#include "IExecutionEngine.h"
#include "IMarketDataFeed.h"
#include "Metrics.h"

#include <iostream>
#include <algorithm>

BacktestEngine::BacktestEngine(std::unique_ptr<ITradingStrategy> strategy,
                               std::unique_ptr<IExecutionEngine> exec,
                               std::unique_ptr<IMarketDataFeed> feed,
                               double initialCash)
  : strategy_(std::move(strategy)),
    exec_(std::move(exec)),
    feed_(std::move(feed)),
    portfolio_(initialCash)
{
}

void BacktestEngine::placeOrder(const Order &order)
{
  pendingOrders_.push_back(order);
}

void BacktestEngine::run()
{
  if(!strategy_ || !exec_ || !feed_)
  {
    std::cerr << "BacktestEngine not properly initialized.\n";
    return;
  }

  equityCurve_.clear();
  equityCurve_.reserve(1024); // arbitrary; avoids some reallocs

  strategy_->onStart(*this);

  while(feed_->hasNext())
  {
    const Candle &bar = feed_->next();
    pendingOrders_.clear();

    // Strategy decides what to do on this bar
    strategy_->onBar(feed_->currentIndex(), bar, *this);

    // Execute any orders placed during onBar
    if(!pendingOrders_.empty())
    {
      exec_->execute(pendingOrders_, bar, portfolio_);
    }

    // For now, treat equity as just cash. If you want to include
    // unrealized PnL, you’ll need a way to value open positions here.
    double equity = portfolio_.getCash();
    equityCurve_.push_back(equity);
  }

  strategy_->onEnd(*this);

  // -------------------------------
  //   Compute and print metrics
  // -------------------------------
  if(equityCurve_.size() >= 2)
  {
    double totalRet = Metrics::totalReturn(equityCurve_);
    double maxDD = Metrics::maxDrawdown(equityCurve_);

    // Approximate years based on bar count, assuming 252 trading days/year.
    double years = static_cast<double>(equityCurve_.size()) / 252.0;
    if(years <= 0.0)
    {
      years = 1.0 / 252.0; // avoid zero/negative
    }
    double cagr = Metrics::cagr(equityCurve_, years);
    double sharpe = Metrics::sharpe(equityCurve_);

    std::cout << "\n=== Performance Metrics ===\n";
    std::cout << "Bars:         " << equityCurve_.size() << "\n";
    std::cout << "Total return: " << totalRet * 100.0 << " %\n";
    std::cout << "Max drawdown: " << maxDD * 100.0 << " %\n";
    std::cout << "CAGR:         " << cagr * 100.0 << " % (approx, " << years << " years)\n";
    std::cout << "Sharpe:       " << sharpe << "\n";
  }
  else
  {
    std::cout << "\nNot enough data to compute performance metrics.\n";
  }
}
