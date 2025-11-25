#pragma once

#include <vector>
#include <string>
#include "TradingTypes.hpp"
#include "Portfolio.hpp"

class Metrics {
public:
    void recordStep(const Portfolio& p, const std::string& ts);
    Report computeReport() const;

private:
    std::vector<Snapshot> snapshots_;
};
