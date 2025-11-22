#pragma once

#include "Types.h"
#include <vector>
#include <memory> // <- add this

class Portfolio;
struct Candle;

class ExecutionEngine_I
{
public:
  virtual ~ExecutionEngine_I() = default;

  virtual void execute(const std::vector<Order> &orders,
                       const Candle &bar,
                       Portfolio &portfolio)
    = 0;
};

// Factory declaration
std::unique_ptr<ExecutionEngine_I> makeSimpleExecutionEngine();
