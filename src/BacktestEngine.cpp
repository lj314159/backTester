#include "BacktestEngine.h"
#include "ITradingStrategy.h"
#include "IExecutionEngine.h"
#include "IMarketDataFeed.h"

BacktestEngine::BacktestEngine(std::unique_ptr<ITradingStrategy> strategy,
                               std::unique_ptr<IExecutionEngine> exec,
                               std::unique_ptr<IMarketDataFeed> feed,
                               double initialCash)
    : strategy_(std::move(strategy)),
      exec_(std::move(exec)),
      feed_(std::move(feed)),
      portfolio_(initialCash)
{}

void BacktestEngine::placeOrder(const Order& order) {
    pendingOrders_.push_back(order);
}

void BacktestEngine::run() {
    if (!strategy_ || !exec_ || !feed_) return;

    strategy_->onStart(*this);

    while (feed_->hasNext()) {
        const Candle& bar = feed_->next();
        pendingOrders_.clear();

        strategy_->onBar(feed_->currentIndex(), bar, *this);

        if (!pendingOrders_.empty()) {
            exec_->execute(pendingOrders_, bar, portfolio_);
        }
    }

    strategy_->onEnd(*this);
}
