#pragma once

#include "Types.h"
#include <vector>
#include <memory> // <- add this

class Portfolio;
struct Candle;

class IExecutionEngine
{
public:
  virtual ~IExecutionEngine() = default;

  virtual void execute(const std::vector<Order> &orders,
                       const Candle &bar,
                       Portfolio &portfolio)
    = 0;
};

// Factory declaration
std::unique_ptr<IExecutionEngine> makeSimpleExecutionEngine();
