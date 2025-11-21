#include "IExecutionEngine.h"
#include "Portfolio.h"
#include <memory>

class SimpleExecutionEngine : public IExecutionEngine
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

std::unique_ptr<IExecutionEngine> makeSimpleExecutionEngine()
{
  return std::make_unique<SimpleExecutionEngine>();
}
