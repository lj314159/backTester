#include <cstdlib>
#include <fstream>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>

#include <nlohmann/json.hpp>

#include "BacktestEngine.hpp"
#include "feed/AlphaVantageFeed.hpp"
#include "exec/SimpleExecutionEngine.hpp"
#include "Strategy_I.hpp"
#include "StrategyFactory.hpp" // <-- new

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
  try
  {
    std::string configPath = "config.json";
    if(argc > 1)
    {
      configPath = argv[1];
    }

    json cfg = loadConfig(configPath);

    // Top-level config
    std::string symbol = cfg.at("asset").get<std::string>();
    double initialCash = cfg.at("initial_cash").get<double>();

    // Data config
    const auto &dataCfg = cfg.at("data");
    std::string provider = dataCfg.at("provider").get<std::string>();
    std::string interval = dataCfg.at("interval").get<std::string>();
    int lookbackBars = dataCfg.at("lookback_bars").get<int>();

    std::unique_ptr<DataFeed_I> feed;

    if(provider == "alpha_vantage")
    {
      if(interval != "daily")
      {
        std::cerr << "WARNING: only 'daily' supported; got '"
                  << interval << "'. Using daily.\n";
      }

      const char *keyEnv = std::getenv("ALPHAVANTAGE_API_KEY");
      if(!keyEnv)
      {
        throw std::runtime_error("ALPHAVANTAGE_API_KEY is not set");
      }
      std::string apiKey = keyEnv;

      feed = makeAlphaVantageFeed(apiKey, symbol, lookbackBars);
    }
    else
    {
      throw std::runtime_error("Unsupported data provider: " + provider);
    }

    // Strategy config
    const auto &stratCfg = cfg.at("strategy");
    std::string stratName = stratCfg.at("name").get<std::string>();

    // Let the factory decide which concrete strategy to build
    std::unique_ptr<Strategy_I> strategy = createStrategy(symbol, stratCfg);

    auto exec = std::make_unique<SimpleExecutionEngine>();

    std::cout << "Running backtest...\n";
    std::cout << "  Strategy: " << stratName << "\n";
    std::cout << "  Symbol:   " << symbol << "\n";
    std::cout << "  Cash:     " << initialCash << "\n";

    BacktestEngine engine(std::move(strategy),
                          std::move(exec),
                          std::move(feed),
                          initialCash);

    Report r = engine.run();

    double finalEquity = engine.portfolio().getEquity();

    std::cout << "\n===== Backtest Results =====\n";
    std::cout << "Initial equity: " << initialCash << "\n";
    std::cout << "Final equity:   " << finalEquity << "\n";
    std::cout << "Total return:   " << r.totalReturn * 100.0 << "%\n";
    std::cout << "CAGR:           " << r.cagr * 100.0 << "%\n";
    std::cout << "Sharpe:         " << r.sharpe << "\n";
    std::cout << "Max drawdown:   " << r.maxDrawdown * 100.0 << "%\n";

    return 0;
  }
  catch(const std::exception &ex)
  {
    std::cerr << "ERROR: " << ex.what() << "\n";
    return 1;
  }
}
