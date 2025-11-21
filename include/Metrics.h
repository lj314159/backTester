#pragma once

#include <vector>

class Metrics {
public:
    // (equityCurve.back() - equityCurve.front()) / equityCurve.front()
    static double totalReturn(const std::vector<double>& curve);

    // CAGR given an explicit number of years
    static double cagr(const std::vector<double>& curve, double years);

    // Max drawdown as a fraction (e.g. 0.15 = 15%)
    static double maxDrawdown(const std::vector<double>& curve);

    // Very basic Sharpe ratio:
    // - computes returns between equity points
    // - annualizes using sqrt(252)
    // - riskFree is per-period (daily) rate
    static double sharpe(const std::vector<double>& curve, double riskFree = 0.0);
};
