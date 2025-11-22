#include "BacktestEngine.h"
#include "ExecutionEngine_I.h"
#include "MarketDataFeed_I.h"
#include "StrategyFactory.h"
#include "AlphaVantageFeed.h"
#include "Types.h"

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>

#include <nlohmann/json.hpp>

using nlohmann::json;

static json loadConfig(const std::string &path)
{
  std::ifstream in(path);
  if(!in)
  {
    throw std::runtime_error("Failed to open config file: " + path);
  }

  json cfg;
  in >> cfg;
  return cfg;
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

  json cfg;
  try
  {
    cfg = loadConfig(configPath);
  }
  catch(const std::exception &ex)
  {
    std::cerr << "Error loading config: " << ex.what() << "\n";
    return 1;
  }

  // -------------------------------------------------
  //  3. Validate basic config fields
  // -------------------------------------------------
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

  // Data provider (only alpha_vantage supported for now).
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

  // Lookback bars: how many most-recent candles to use.
  // Default: 100 (previous hard-coded behavior).
  int lookbackBars = 100;
  if(cfg.contains("data") && cfg["data"].contains("lookback_bars"))
  {
    lookbackBars = cfg["data"]["lookback_bars"].get<int>();
  }

  try
  {
    // -------------------------------------------------
    //  4. Build data feed
    // -------------------------------------------------
    std::cout << "Fetching " << symbol
              << " from Alpha Vantage..." << std::endl;

    auto feed = makeAlphaVantageFeed(apiKey, symbol, lookbackBars);

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
