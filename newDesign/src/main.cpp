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

using nlohmann::json;

// Forward declarations of factory functions implemented in strategy .cpp files
std::unique_ptr<Strategy_I>
makeTrendRsiStrategy(const std::string& symbol,
                     int period,
                     double overbought,
                     double oversold,
                     int trendWindow);

std::unique_ptr<Strategy_I>
makeSmaCrossoverStrategy(const std::string& symbol,
                         int shortPeriod,
                         int longPeriod);

std::unique_ptr<Strategy_I>
makeBreakoutStrategy(const std::string& symbol,
                     std::size_t lookbackWindow);

static json loadConfig(const std::string& path)
{
    std::ifstream in(path);
    if (!in) {
        throw std::runtime_error("Failed to open config file: " + path);
    }
    json cfg;
    in >> cfg;
    return cfg;
}

int main(int argc, char** argv)
{
    try {
        std::string configPath = "config.json";
        if (argc > 1) {
            configPath = argv[1];
        }

        json cfg = loadConfig(configPath);

        std::string symbol      = cfg.at("asset").get<std::string>();
        double initialCash      = cfg.at("initial_cash").get<double>();

        const auto& dataCfg     = cfg.at("data");
        std::string provider    = dataCfg.at("provider").get<std::string>();
        std::string interval    = dataCfg.at("interval").get<std::string>();
        int lookbackBars        = dataCfg.at("lookback_bars").get<int>();

        std::unique_ptr<DataFeed_I> feed;

        if (provider == "alpha_vantage") {
            if (interval != "daily") {
                std::cerr << "WARNING: only 'daily' supported; got '"
                          << interval << "'. Using daily.\n";
            }

            const char* keyEnv = std::getenv("ALPHAVANTAGE_API_KEY");
            if (!keyEnv) {
                throw std::runtime_error("ALPHAVANTAGE_API_KEY is not set");
            }
            std::string apiKey = keyEnv;

            feed = makeAlphaVantageFeed(apiKey, symbol, lookbackBars);
        } else {
            throw std::runtime_error("Unsupported data provider: " + provider);
        }

        const auto& stratCfg  = cfg.at("strategy");
        std::string stratName = stratCfg.at("name").get<std::string>();

        std::unique_ptr<Strategy_I> strategy;

        if (stratName == "trend_rsi") {
            const auto& params = stratCfg.at("params");
            int period         = params.at("period").get<int>();
            double overbought  = params.at("overbought").get<double>();
            double oversold    = params.at("oversold").get<double>();
            int trendWindow    = params.at("trend_window").get<int>();

            strategy = makeTrendRsiStrategy(symbol, period, overbought, oversold, trendWindow);
        } else if (stratName == "sma_crossover") {
            const auto& params = stratCfg.at("params");
            int shortP         = params.at("short_period").get<int>();
            int longP          = params.at("long_period").get<int>();

            strategy = makeSmaCrossoverStrategy(symbol, shortP, longP);
        } else if (stratName == "breakout") {
            const auto& params = stratCfg.at("params");
            std::size_t lb     = params.at("lookback_window").get<std::size_t>();

            strategy = makeBreakoutStrategy(symbol, lb);
        } else {
            throw std::runtime_error("Unsupported strategy name: " + stratName);
        }

        auto exec = std::make_unique<SimpleExecutionEngine>();

        std::cout << "Running backtest...\n";
        std::cout << "  Strategy: " << stratName   << "\n";
        std::cout << "  Symbol:   " << symbol      << "\n";
        std::cout << "  Cash:     " << initialCash << "\n";

        BacktestEngine engine(std::move(strategy),
                              std::move(exec),
                              std::move(feed),
                              initialCash);

        Report r = engine.run();

        double finalEquity = engine.portfolio().getEquity();

        std::cout << "\n===== Backtest Results =====\n";
        std::cout << "Initial equity: " << initialCash               << "\n";
        std::cout << "Final equity:   " << finalEquity               << "\n";
        std::cout << "Total return:   " << r.totalReturn * 100.0     << "%\n";
        std::cout << "CAGR:           " << r.cagr * 100.0            << "%\n";
        std::cout << "Sharpe:         " << r.sharpe                  << "\n";
        std::cout << "Max drawdown:   " << r.maxDrawdown * 100.0     << "%\n";

        return 0;
    } catch (const std::exception& ex) {
        std::cerr << "ERROR: " << ex.what() << "\n";
        return 1;
    }
}
