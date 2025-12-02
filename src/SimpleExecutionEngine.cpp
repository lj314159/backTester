#include "exec/SimpleExecutionEngine.hpp"

std::optional<Fill>
SimpleExecutionEngine::execute(const Order &order, const Candle &bar)
{
  if(order.type != OrderType::Market)
  {
    return std::nullopt;
  }

  Fill f;
  f.symbol = order.symbol;
  f.quantity = order.quantity;
  f.side = order.side;
  f.price = bar.close;
  f.fees = 0.0;
  f.timestamp = bar.timestamp;
  return f;
}
