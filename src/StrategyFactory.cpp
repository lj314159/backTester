#include "StrategyFactory.h"
#include "ITradingStrategy.h"

#include <stdexcept>

using nlohmann::json;

std::unique_ptr<ITradingStrategy>
makeStrategyFromConfig(const json &strategyJson,
                       const std::string &symbol)
{
  if(!strategyJson.contains("name"))
  {
    throw std::runtime_error("Config error: 'strategy.name' is missing");
  }

  std::string name = strategyJson.at("name").get<std::string>();

  json params = json::object();
  if(strategyJson.contains("params"))
  {
    params = strategyJson.at("params");
  }

  if(name == "simple_sma")
  {
    std::size_t window =
      params.value("window", static_cast<std::size_t>(10));
    return makeSimpleSMAStrategy(symbol, window);
  }
  else if(name == "sma_crossover")
  {
    std::size_t fastWindow =
      params.value("fast_window", static_cast<std::size_t>(10));
    std::size_t slowWindow =
      params.value("slow_window", static_cast<std::size_t>(30));
    return makeSmaCrossoverStrategy(symbol, fastWindow, slowWindow);
  }
  else if(name == "rsi_reversion")
  {
    std::size_t period =
      params.value("period", static_cast<std::size_t>(14));
    double overbought = params.value("overbought", 70.0);
    double oversold   = params.value("oversold", 30.0);
    return makeRsiReversionStrategy(symbol, period, overbought, oversold);
  }
  else if(name == "breakout")
  {
    std::size_t lookback =
      params.value("lookback_window", static_cast<std::size_t>(20));
    return makeBreakoutStrategy(symbol, lookback);
  }
  else if(name == "trend_rsi")
  {
    std::size_t period =
      params.value("period", static_cast<std::size_t>(14));
    double overbought = params.value("overbought", 60.0);
    double oversold   = params.value("oversold", 40.0);
    std::size_t trendWindow =
      params.value("trend_window", static_cast<std::size_t>(50));
    return makeTrendRsiStrategy(symbol,
                                period,
                                overbought,
                                oversold,
                                trendWindow);
  }

  throw std::runtime_error("Config error: unknown strategy name '" + name + "'");
}
