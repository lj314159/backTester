#pragma once

#include <string>

struct Candle
{
  std::string timestamp; // e.g. "2025-01-01 09:30"
  double open = 0.0;
  double high = 0.0;
  double low = 0.0;
  double close = 0.0;
  double volume = 0.0;
};

enum class OrderSide
{
  Buy,
  Sell
};

struct Order
{
  std::string symbol;
  OrderSide side = OrderSide::Buy;
  int quantity = 0;
};
