#pragma once

#include "Types.h"
#include <string>
#include <unordered_map>

struct Position
{
  std::string symbol;
  int quantity = 0;
  double avgPrice = 0.0;
};

class Portfolio
{
public:
  explicit Portfolio(double initialCash = 0.0);

  void applyFill(const Order &order, double fillPrice);

  double getCash() const { return cash_; }
  int getPositionQty(const std::string &symbol) const;
  double getPositionValue(const std::string &symbol, double lastPrice) const;

private:
  double cash_;
  std::unordered_map<std::string, Position> positions_;
};
