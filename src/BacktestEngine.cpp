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
                               double initialCash,
                               std::string assetSymbol)
  : strategy_(std::move(strategy)),
    exec_(std::move(exec)),
    feed_(std::move(feed)),
    portfolio_(initialCash),
    initialCash_(initialCash),
    startDate_(),
    endDate_(),
    asset_(std::move(assetSymbol))
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

  startDate_.clear();
  endDate_.clear();

  strategy_->onStart(*this);

  while(feed_->hasNext())
  {
    const Candle &bar = feed_->next();
    pendingOrders_.clear();

    // Record start/end dates for duration metrics.
    if(equityCurve_.empty())
    {
      startDate_ = bar.timestamp;
    }
    endDate_ = bar.timestamp;

    // Strategy decides what to do on this bar.
    strategy_->onBar(feed_->currentIndex(), bar, *this);

    // Execute any orders placed during onBar.
    if(!pendingOrders_.empty())
    {
      exec_->execute(pendingOrders_, bar, portfolio_);
    }

    // For now, treat equity as just cash. If you want to include
    // unrealized PnL, you will need a way to value open positions here.
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

    // P/L measures.
    double plPercent = totalRet * 100.0;
    double plAmount = portfolio_.getCash() - initialCash_;

    // Approximate years based on bar count, assuming 252 trading days/year.
    double bars = static_cast<double>(equityCurve_.size());
    double years = bars / 252.0;
    if(years <= 0.0)
    {
      years = 1.0 / 252.0; // avoid zero/negative
    }

    double cagr = Metrics::cagr(equityCurve_, years);
    double sharpe = Metrics::sharpe(equityCurve_);

    std::cout << "\n=== Performance Metrics ===\n";
    std::cout << "Asset:         " << asset_ << "\n";
    std::cout << "Bars:          " << static_cast<int>(bars) << "\n";
    std::cout << "Start date:    " << startDate_ << "\n";
    std::cout << "End date:      " << endDate_ << "\n";
    std::cout << "Duration:      " << years << " years (approx)\n";
    std::cout << "P/L Amount:    " << plAmount << "\n";
    std::cout << "P/L Percent:   " << plPercent << " %\n";
    std::cout << "Total return:  " << totalRet * 100.0 << " %\n";
    std::cout << "Max drawdown:  " << maxDD * 100.0 << " %\n";
    std::cout << "CAGR:          " << cagr * 100.0 << " %\n";
    std::cout << "Sharpe:        " << sharpe << "\n";
  }
  else
  {
    std::cout << "\nNot enough data to compute performance metrics.\n";
  }
}
