#pragma once

#include <unordered_map>
#include <string>
#include "TradingTypes.hpp"

class Portfolio {
public:
    explicit Portfolio(double initialCash = 0.0)
        : cash_(initialCash) {}

    void applyFill(const Fill& f);
    void markToMarket(const Candle& bar);

    double getEquity() const;
    double getCash()   const { return cash_; }

    const Position* getPosition(const std::string& symbol) const;

private:
    double cash_{};
    std::unordered_map<std::string, Position> positions_;
};
