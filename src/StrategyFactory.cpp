#include "StrategyFactory.hpp"
#include "Strategy_I.hpp"
#include <stdexcept>

// Forward declarations of concrete strategy factories
std::unique_ptr<Strategy_I>
makeTrendRsiStrategy(const std::string &symbol,
                     int period,
                     double overbought,
                     double oversold,
                     int trendWindow);

// create new strategy demo
/*
std::unique_ptr<Strategy_I>
makeMyNewStrategy(const std::string &symbol,
                  int p1,
                  double p2);
*/

std::unique_ptr<Strategy_I>
makeSmaCrossoverStrategy(const std::string &symbol,
                         int shortPeriod,
                         int longPeriod);

std::unique_ptr<Strategy_I>
makeBreakoutStrategy(const std::string &symbol,
                     std::size_t lookbackWindow);

std::unique_ptr<Strategy_I>
makeZScoreMeanReversionStrategy(const std::string &symbol,
                                int lookback,
                                double entryZ,
                                double exitZ);

// register strategy names for config.json
std::unique_ptr<Strategy_I>
createStrategy(const std::string &symbol,
               const nlohmann::json &stratCfg)
{
  std::string stratName = stratCfg.at("name").get<std::string>();
  const auto &params = stratCfg.at("params");

  if(stratName == "trend_rsi")
  {
    int period = params.at("period").get<int>();
    double overbought = params.at("overbought").get<double>();
    double oversold = params.at("oversold").get<double>();
    int trendWindow = params.at("trend_window").get<int>();

    return makeTrendRsiStrategy(symbol, period, overbought, oversold, trendWindow);
  }
  // register strategy name demo
  /*
  else if (stratName == "my_new_strategy")
  {
      int p1 = params.at("p1").get<int>();
      double p2 = params.at("p2").get<double>();

      return makeMyNewStrategy(symbol, p1, p2);
  }
  */
  else if(stratName == "sma_crossover")
  {
    int shortP = params.at("short_period").get<int>();
    int longP = params.at("long_period").get<int>();

    return makeSmaCrossoverStrategy(symbol, shortP, longP);
  }
  else if(stratName == "breakout")
  {
    std::size_t lb = params.at("lookback_window").get<std::size_t>();
    return makeBreakoutStrategy(symbol, lb);
  }

  else if(stratName == "mean_reversion_zscore")
  {
    int lookback = params.at("lookback").get<int>();
    double entryZ = params.at("entry_zscore").get<double>();
    double exitZ = params.at("exit_zscore").get<double>();

    return makeZScoreMeanReversionStrategy(symbol, lookback, entryZ, exitZ);
  }

  throw std::runtime_error("Unsupported strategy name: " + stratName);
}
