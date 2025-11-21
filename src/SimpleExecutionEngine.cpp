#include "IExecutionEngine.h"
#include "Portfolio.h"

class SimpleExecutionEngine : public IExecutionEngine {
public:
    void execute(const std::vector<Order>& orders,
                 const Candle& bar,
                 Portfolio& portfolio) override
    {
        for (const auto& ord : orders) {
            portfolio.applyFill(ord, bar.close);
        }
    }
};

// Factory helper so main.cpp can create it easily.
std::unique_ptr<IExecutionEngine> makeSimpleExecutionEngine() {
    return std::make_unique<SimpleExecutionEngine>();
}
