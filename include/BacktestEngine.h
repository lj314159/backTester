#pragma once

#include "Types.h"
#include "Portfolio.h"
#include <memory>
#include <vector>

class ITradingStrategy;
class IExecutionEngine;
class IMarketDataFeed;

class BacktestEngine {
public:
    BacktestEngine(std::unique_ptr<ITradingStrategy> strategy,
                   std::unique_ptr<IExecutionEngine> exec,
                   std::unique_ptr<IMarketDataFeed> feed,
                   double initialCash);

    void run();

    // Called by strategies:
    void placeOrder(const Order& order);

    Portfolio& portfolio() { return portfolio_; }
    const Portfolio& portfolio() const { return portfolio_; }

private:
    std::unique_ptr<ITradingStrategy> strategy_;
    std::unique_ptr<IExecutionEngine> exec_;
    std::unique_ptr<IMarketDataFeed>  feed_;
    Portfolio                         portfolio_;

    std::vector<Order> pendingOrders_;

    friend class SimpleSMAStrategy; // optional, for convenience
};
