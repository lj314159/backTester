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
  equityCurve_.reserve(1024);

  strategy_->onStart(*this);

  while(feed_->hasNext())
  {
    const Candle &bar = feed_->next();
    pendingOrders_.clear();

    strategy_->onBar(feed_->currentIndex(), bar, *this);

    if(!pendingOrders_.empty())
    {
      exec_->execute(pendingOrders_, bar, portfolio_);
    }

    double equity = portfolio_.getCash();
    equityCurve_.push_back(equity);
  }

  strategy_->onEnd(*this);

  if(equityCurve_.size() >= 2)
  {
    double totalRet = Metrics::totalReturn(equityCurve_);
    double maxDD = Metrics::maxDrawdown(equityCurve_);

    double years = static_cast<double>(equityCurve_.size()) / 252.0;
    if(years <= 0.0)
    {
      years = 1.0 / 252.0;
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
