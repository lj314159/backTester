#include "Portfolio.hpp"
#include <algorithm>
#include <cmath>

void Portfolio::applyFill(const Fill& f)
{
    int dir = 0;
    switch (f.side) {
    case OrderSide::Buy:
    case OrderSide::Cover:
        dir = +1;
        break;
    case OrderSide::Sell:
    case OrderSide::Short:
        dir = -1;
        break;
    }

    int signedQty = dir * f.quantity;

    auto& pos = positions_[f.symbol];
    if (pos.symbol.empty()) {
        pos.symbol = f.symbol;
    }

    if (pos.quantity == 0) {
        pos.quantity = signedQty;
        pos.avgPrice = f.price;
    } else {
        if ((pos.quantity > 0 && signedQty > 0) ||
            (pos.quantity < 0 && signedQty < 0)) {
            double oldValue = pos.avgPrice * std::abs(pos.quantity);
            double newValue = f.price * std::abs(signedQty);
            int newQty      = pos.quantity + signedQty;
            if (newQty != 0) {
                pos.avgPrice = (oldValue + newValue) / std::abs(newQty);
            }
            pos.quantity = newQty;
        } else {
            pos.quantity += signedQty;
            if (pos.quantity == 0) {
                pos.avgPrice = 0.0;
            }
        }
    }

    double tradeValue = f.price * f.quantity;
    if (dir > 0) {
        cash_ -= tradeValue;
        cash_ -= f.fees;
    } else {
        cash_ += tradeValue;
        cash_ -= f.fees;
    }
}

void Portfolio::markToMarket(const Candle& bar)
{
    auto it = positions_.find(bar.symbol);
    if (it != positions_.end()) {
        auto& pos = it->second;
        if (pos.quantity != 0) {
            pos.unrealizedPnL = (bar.close - pos.avgPrice) * pos.quantity;
        } else {
            pos.unrealizedPnL = 0.0;
        }
    }
}

double Portfolio::getEquity() const
{
    double unreal = 0.0;
    for (const auto& [sym, pos] : positions_) {
        unreal += pos.unrealizedPnL;
    }
    return cash_ + unreal;
}

const Position* Portfolio::getPosition(const std::string& symbol) const
{
    auto it = positions_.find(symbol);
    if (it != positions_.end()) {
        return &it->second;
    }
    return nullptr;
}
