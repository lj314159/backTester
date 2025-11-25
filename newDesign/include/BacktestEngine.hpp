#pragma once

#include <memory>
#include <vector>

#include "Portfolio.hpp"
#include "Metrics.hpp"
#include "Strategy_I.hpp"
#include "DataFeed_I.hpp"
#include "ExecutionEngine_I.hpp"
#include "TradingTypes.hpp"

class BacktestEngine {
public:
    BacktestEngine(std::unique_ptr<Strategy_I>        strategy,
                   std::unique_ptr<ExecutionEngine_I> exec,
                   std::unique_ptr<DataFeed_I>        feed,
                   double                             initialCash);

    void placeOrder(const Order& o);

    Report run();

    Portfolio&       portfolio()       { return portfolio_; }
    const Portfolio& portfolio() const { return portfolio_; }

private:
    std::vector<Order> pendingOrders_;

    std::unique_ptr<Strategy_I>        strategy_;
    std::unique_ptr<ExecutionEngine_I> exec_;
    std::unique_ptr<DataFeed_I>        feed_;
    Portfolio                          portfolio_;
    Metrics                            metrics_;
};
