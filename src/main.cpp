#include "BacktestEngine.h"
#include "ITradingStrategy.h"
#include "IExecutionEngine.h"
#include "IMarketDataFeed.h"
#include "AlphaVantageFeed.h"
#include "Types.h"

#include <cstdlib>
#include <iostream>
#include <string>

int main()
{
  const char *keyEnv = std::getenv("ALPHAVANTAGE_API_KEY");
  if(keyEnv == nullptr)
  {
    std::cerr << "ALPHAVANTAGE_API_KEY not set\n";
    return 1;
  }

  const std::string apiKey = keyEnv;
  const std::string symbol = "AAPL";

  try
  {
    std::cout << "Fetching " << symbol
              << " from Alpha Vantage..." << std::endl;

    // Market data feed from Alpha Vantage (daily data).
    auto feed = makeAlphaVantageFeed(apiKey, symbol);

    std::cout << "Finished fetching " << symbol
              << " from Alpha Vantage." << std::endl;

    // Simple execution engine: fills at bar.close.
    auto exec = makeSimpleExecutionEngine();

    // Simple SMA-based trading strategy.
    auto strategy = makeSimpleSMAStrategy(symbol, 10);

    // Backtest engine with initial cash and tested asset symbol.
    BacktestEngine engine(std::move(strategy),
                          std::move(exec),
                          std::move(feed),
                          100000.0,
                          symbol);

    engine.run();

    std::cout << "Backtest complete. Final cash: "
              << engine.portfolio().getCash()
              << "\n";
  }
  catch(const std::exception &ex)
  {
    std::cerr << "Failed fetching " << symbol
              << " from Alpha Vantage.\n";
    std::cerr << "Error: " << ex.what() << "\n";
    return 1;
  }

  return 0;
}
