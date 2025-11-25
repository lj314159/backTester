#include "BacktestEngine.hpp"

BacktestEngine::BacktestEngine(std::unique_ptr<Strategy_I> strategy,
                               std::unique_ptr<ExecutionEngine_I> exec,
                               std::unique_ptr<DataFeed_I> feed,
                               double initialCash)
  : strategy_(std::move(strategy)),
    exec_(std::move(exec)),
    feed_(std::move(feed)),
    portfolio_(initialCash)
{
}

void BacktestEngine::placeOrder(const Order &o)
{
  pendingOrders_.push_back(o);
}

Report BacktestEngine::run()
{
  std::size_t index = 0;

  strategy_->onStart(*this);

  while(feed_->hasNext())
  {
    const Candle &bar = feed_->next();

    // Strategy decides what to do; calls engine.placeOrder(...)
    strategy_->onBar(index, bar, *this);

    // Execute pending orders
    for(const auto &o : pendingOrders_)
    {
      if(auto fill = exec_->execute(o, bar))
      {
        portfolio_.applyFill(*fill);
      }
    }
    pendingOrders_.clear();

    // Mark-to-market and record metrics
    portfolio_.markToMarket(bar);
    metrics_.recordStep(portfolio_, bar.timestamp);

    ++index;
  }

  strategy_->onEnd(*this);

  return metrics_.computeReport();
}
