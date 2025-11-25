#pragma once

#include <string>

// ============================================================
// Enums
// ============================================================

enum class OrderSide
{
  Buy,   // open / increase long, or close short
  Sell,  // close / reduce long, or open short
  Short, // explicitly open / increase short
  Cover  // explicitly close / reduce short
};

enum class OrderType
{
  Market,
  Limit,
  Stop,
  StopLimit,
  MarketOnOpen,
  MarketOnClose
};

// ============================================================
// Core data types (POD-style structs)
// ============================================================

struct Candle
{
  std::string timestamp;
  std::string symbol;
  double open{};
  double high{};
  double low{};
  double close{};
  double volume{};
};

struct Order
{
  std::string symbol;
  int quantity{}; // positive; side controls direction
  OrderSide side{ OrderSide::Buy };
  OrderType type{ OrderType::Market };
  double limitPrice{};
  double stopPrice{};
};

struct Fill
{
  std::string symbol;
  int quantity{};
  OrderSide side{ OrderSide::Buy };
  double price{};
  double fees{};
  std::string timestamp;
};

struct Position
{
  std::string symbol;
  int quantity{}; // >0 long, <0 short
  double avgPrice{};
  double unrealizedPnL{};
};

struct Snapshot
{
  std::string timestamp;
  double equity{};
  double cash{};
  double realizedPnL{};
  double unrealizedPnL{};
};

struct Report
{
  double totalReturn{};
  double maxDrawdown{};
  double sharpe{};
  double cagr{};
};
