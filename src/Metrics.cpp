#include "Metrics.h"
#include <cmath>
#include <algorithm>

double Metrics::totalReturn(const std::vector<double>& curve) {
    if (curve.size() < 2 || curve.front() <= 0.0) {
        return 0.0;
    }
    return (curve.back() - curve.front()) / curve.front();
}

double Metrics::maxDrawdown(const std::vector<double>& curve) {
    if (curve.empty()) return 0.0;

    double peak = curve.front();
    double maxDD = 0.0;

    for (double v : curve) {
        peak = std::max(peak, v);
        double dd = (peak - v) / peak;  // drawdown as fraction from peak
        maxDD = std::max(maxDD, dd);
    }
    return maxDD;
}

double Metrics::cagr(const std::vector<double>& curve, double years) {
    if (curve.size() < 2 || curve.front() <= 0.0 || years <= 0.0) {
        return 0.0;
    }
    double finalOverInitial = curve.back() / curve.front();
    return std::pow(finalOverInitial, 1.0 / years) - 1.0;
}

double Metrics::sharpe(const std::vector<double>& curve, double riskFree) {
    if (curve.size() < 2) return 0.0;

    // Compute per-step returns
    std::vector<double> rets;
    rets.reserve(curve.size() - 1);

    for (std::size_t i = 1; i < curve.size(); ++i) {
        if (curve[i - 1] <= 0.0) continue;
        double r = (curve[i] - curve[i - 1]) / curve[i - 1];
        rets.push_back(r);
    }

    if (rets.empty()) return 0.0;

    double mean = 0.0;
    for (double r : rets) mean += r;
    mean /= static_cast<double>(rets.size());

    double var = 0.0;
    for (double r : rets) {
        double diff = r - mean;
        var += diff * diff;
    }
    var /= static_cast<double>(rets.size());
    double stdev = std::sqrt(var);

    if (stdev == 0.0) return 0.0;

    // Annualize assuming 252 trading days
    double excess = mean - riskFree;
    return (excess / stdev) * std::sqrt(252.0);
}
