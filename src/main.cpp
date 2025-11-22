#include "BacktestEngine.h"
#include "ITradingStrategy.h"
#include "IExecutionEngine.h"
#include "IMarketDataFeed.h"
#include "AlphaVantageFeed.h"
#include "Types.h"

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>

#include <nlohmann/json.hpp>

using nlohmann::json;

// Helper: choose and build a strategy from JSON config.
static std::unique_ptr<ITradingStrategy>
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
    std::size_t window = params.value("window", static_cast<std::size_t>(10));
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

  throw std::runtime_error("Config error: unknown strategy name '" + name + "'");
}

int main(int argc, char **argv)
{
  // -------------------------------------------------
  //  1. Get API key from environment
  // -------------------------------------------------
  const char *keyEnv = std::getenv("ALPHAVANTAGE_API_KEY");
  if(keyEnv == nullptr)
  {
    std::cerr << "ALPHAVANTAGE_API_KEY not set\n";
    return 1;
  }
  const std::string apiKey = keyEnv;

  // -------------------------------------------------
  //  2. Determine config file path
  //     Default: ./config.json
  //     Or:      ./backtest_engine myconfig.json
  // -------------------------------------------------
  std::string configPath = "config.json";
  if(argc > 1)
  {
    configPath = argv[1];
  }

  // -------------------------------------------------
  //  3. Load JSON config
  // -------------------------------------------------
  json cfg;
  {
    std::ifstream in(configPath);
    if(!in)
    {
      std::cerr << "Failed to open config file: " << configPath << "\n";
      return 1;
    }
    try
    {
      in >> cfg;
    }
    catch(const std::exception &ex)
    {
      std::cerr << "Error parsing config JSON in " << configPath
                << ": " << ex.what() << "\n";
      return 1;
    }
  }

  // Basic required fields.
  if(!cfg.contains("asset"))
  {
    std::cerr << "Config error: 'asset' field is required\n";
    return 1;
  }

  if(!cfg.contains("strategy"))
  {
    std::cerr << "Config error: 'strategy' section is required\n";
    return 1;
  }

  std::string symbol = cfg.at("asset").get<std::string>();
  double initialCash = cfg.value("initial_cash", 100000.0);

  // -------------------------------------------------
  //  4. Build data feed (currently: Alpha Vantage only)
  // -------------------------------------------------
  std::string provider = "alpha_vantage";
  if(cfg.contains("data") && cfg["data"].contains("provider"))
  {
    provider = cfg["data"]["provider"].get<std::string>();
  }

  if(provider != "alpha_vantage")
  {
    std::cerr << "Config error: only 'alpha_vantage' provider is supported "
                 "in this version. Got: "
              << provider << "\n";
    return 1;
  }

  try
  {
    std::cout << "Fetching " << symbol
              << " from Alpha Vantage..." << std::endl;

    auto feed = makeAlphaVantageFeed(apiKey, symbol);

    std::cout << "Finished fetching " << symbol
              << " from Alpha Vantage." << std::endl;

    // -------------------------------------------------
    //  5. Build execution engine and strategy
    // -------------------------------------------------
    auto exec = makeSimpleExecutionEngine();

    const json &strategyJson = cfg.at("strategy");
    auto strategy = makeStrategyFromConfig(strategyJson, symbol);

    // -------------------------------------------------
    //  6. Run backtest
    // -------------------------------------------------
    BacktestEngine engine(std::move(strategy),
                          std::move(exec),
                          std::move(feed),
                          initialCash,
                          symbol);

    engine.run();

    std::cout << "Backtest complete. Final cash: "
              << engine.portfolio().getCash()
              << "\n";
  }
  catch(const std::exception &ex)
  {
    std::cerr << "Error during backtest: " << ex.what() << "\n";
    return 1;
  }

  return 0;
}
