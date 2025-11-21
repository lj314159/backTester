#include "BacktestEngine.h"
#include "ITradingStrategy.h"
#include "IExecutionEngine.h"
#include "IMarketDataFeed.h"
#include "Types.h"
#include <vector>
#include <iostream>

int main() {
    // Dummy price series: simple trend up then down
    std::vector<Candle> candles;
    double price = 100.0;
    for (int i = 0; i < 100; ++i) {
        Candle c;
        c.timestamp = "2025-01-01T00:" + std::to_string(i);
        c.open  = price;
        c.high  = price + 1.0;
        c.low   = price - 1.0;
        c.close = price;
        c.volume = 1000.0;
        candles.push_back(c);

        if (i < 50) price += 0.5;
        else        price -= 0.5;
    }

    auto feed     = makeVectorFeed(std::move(candles));
    auto exec     = makeSimpleExecutionEngine();
    auto strategy = makeSimpleSMAStrategy("AAPL", 10);

    BacktestEngine engine(std::move(strategy),
                          std::move(exec),
                          std::move(feed),
                          /*initialCash*/ 100000.0);

    engine.run();

    std::cout << "Backtest complete. Final cash: "
              << engine.portfolio().getCash()
              << "\n";

    return 0;
}
