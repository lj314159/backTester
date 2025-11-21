#include "Portfolio.h"
#include <stdexcept>

Portfolio::Portfolio(double initialCash)
    : cash_(initialCash)
{}

// Very simple fill model: immediate fill at fillPrice, no fees.
void Portfolio::applyFill(const Order& order, double fillPrice) {
    auto& pos = positions_[order.symbol];
    pos.symbol = order.symbol;

    if (order.side == OrderSide::Buy) {
        double cost = fillPrice * order.quantity;
        cash_ -= cost;

        double newQty = pos.quantity + order.quantity;
        if (newQty == 0) {
            pos.quantity = 0;
            pos.avgPrice = 0.0;
        } else {
            pos.avgPrice =
                (pos.avgPrice * pos.quantity + cost) / newQty;
            pos.quantity = static_cast<int>(newQty);
        }
    } else { // Sell
        double proceeds = fillPrice * order.quantity;
        cash_ += proceeds;
        pos.quantity -= order.quantity;
        if (pos.quantity <= 0) {
            pos.quantity = 0;
            pos.avgPrice = 0.0;
        }
    }
}

int Portfolio::getPositionQty(const std::string& symbol) const {
    auto it = positions_.find(symbol);
    if (it == positions_.end()) return 0;
    return it->second.quantity;
}

double Portfolio::getPositionValue(const std::string& symbol,
                                   double lastPrice) const
{
    auto it = positions_.find(symbol);
    if (it == positions_.end()) return 0.0;
    return it->second.quantity * lastPrice;
}
