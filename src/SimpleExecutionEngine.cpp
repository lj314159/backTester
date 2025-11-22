#include "ExecutionEngine_I.h"
#include "Portfolio.h"
#include <memory>

class SimpleExecutionEngine : public ExecutionEngine_I
{
public:
  void execute(const std::vector<Order> &orders,
               const Candle &bar,
               Portfolio &portfolio) override
  {
    for(const auto &ord : orders)
    {
      portfolio.applyFill(ord, bar.close);
    }
  }
};

std::unique_ptr<ExecutionEngine_I> makeSimpleExecutionEngine()
{
  return std::make_unique<SimpleExecutionEngine>();
}
