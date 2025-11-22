#pragma once

#include "Types.h"
#include "Portfolio.h"

#include <memory>
#include <vector>
#include <string>

// Forward declarations of interfaces
class ITradingStrategy;
class IExecutionEngine;
class IMarketDataFeed;

/*
 * BacktestEngine
 * --------------
 * Orchestrates the backtest:
 *  - owns the strategy, execution engine, and data feed
 *  - runs the main simulation loop
 *  - holds the portfolio state
 *  - collects orders placed by the strategy
 *  - builds an equity curve and prints metrics
 */
class BacktestEngine
{
public:
  // Construct a backtest engine with a strategy, execution engine,
  // market data feed, initial cash, and the asset symbol being tested.
  BacktestEngine(std::unique_ptr<ITradingStrategy> strategy,
                 std::unique_ptr<IExecutionEngine> exec,
                 std::unique_ptr<IMarketDataFeed> feed,
                 double initialCash,
                 std::string assetSymbol);

  // Run the full backtest (calls strategy->onStart/onBar/onEnd).
  void run();

  // Called by strategies to request trades.
  void placeOrder(const Order &order);

  // Access to the portfolio (for strategies, main, etc.).
  Portfolio &portfolio()
  {
    return portfolio_;
  }

  const Portfolio &portfolio() const
  {
    return portfolio_;
  }

  // Read-only access to the equity curve for metrics or external analysis.
  const std::vector<double> &equityCurve() const
  {
    return equityCurve_;
  }

private:
  std::unique_ptr<ITradingStrategy> strategy_;
  std::unique_ptr<IExecutionEngine> exec_;
  std::unique_ptr<IMarketDataFeed> feed_;
  Portfolio portfolio_;

  // Orders placed during the current bar, to be passed to the execution engine.
  std::vector<Order> pendingOrders_;

  // Equity curve (portfolio value over time), useful for metrics.
  std::vector<double> equityCurve_;

  // For P/L metrics.
  double initialCash_;

  // For duration metrics.
  std::string startDate_;
  std::string endDate_;

  // Asset symbol being tested (e.g. "AAPL").
  std::string asset_;
};
