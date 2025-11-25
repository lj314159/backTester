#include "Metrics.hpp"
#include <algorithm>
#include <cmath>
#include <vector>

void Metrics::recordStep(const Portfolio &p, const std::string &ts)
{
  Snapshot s;
  s.timestamp = ts;
  s.equity = p.getEquity();
  s.cash = p.getCash();
  snapshots_.push_back(s);
}

Report Metrics::computeReport() const
{
  Report r{};
  if(snapshots_.empty())
  {
    return r;
  }

  const std::size_t n = snapshots_.size();
  double start = snapshots_.front().equity;
  double end = snapshots_.back().equity;

  if(start > 0.0)
  {
    r.totalReturn = (end - start) / start;
  }

  double peak = snapshots_.front().equity;
  double maxDD = 0.0;
  for(const auto &s : snapshots_)
  {
    peak = std::max(peak, s.equity);
    if(peak > 0.0)
    {
      double dd = (peak - s.equity) / peak;
      maxDD = std::max(maxDD, dd);
    }
  }
  r.maxDrawdown = maxDD;

  if(n > 1 && start > 0.0 && end > 0.0)
  {
    std::vector<double> returns;
    returns.reserve(n - 1);
    for(std::size_t i = 1; i < n; ++i)
    {
      double prev = snapshots_[i - 1].equity;
      double curr = snapshots_[i].equity;
      if(prev > 0.0)
      {
        returns.push_back((curr - prev) / prev);
      }
    }

    if(!returns.empty())
    {
      double sum = 0.0;
      for(double v : returns)
      {
        sum += v;
      }
      double mean = sum / static_cast<double>(returns.size());

      double var = 0.0;
      if(returns.size() > 1)
      {
        for(double v : returns)
        {
          double d = v - mean;
          var += d * d;
        }
        var /= static_cast<double>(returns.size() - 1);
      }
      double stdDev = var > 0.0 ? std::sqrt(var) : 0.0;

      constexpr double tradingDaysPerYear = 252.0;

      if(stdDev > 0.0)
      {
        r.sharpe = std::sqrt(tradingDaysPerYear) * (mean / stdDev);
      }

      double years = static_cast<double>(returns.size()) / tradingDaysPerYear;
      if(years > 0.0)
      {
        r.cagr = std::pow(end / start, 1.0 / years) - 1.0;
      }
    }
  }

  return r;
}
