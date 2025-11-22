#pragma once

#include "TradingStrategy_I.h"

#include <memory>
#include <string>
#include <nlohmann/json.hpp>

// Factory function that builds a concrete strategy from
// the "strategy" section of the config and the asset symbol.
std::unique_ptr<TradingStrategy_I>
makeStrategyFromConfig(const nlohmann::json &strategyJson,
                       const std::string &symbol);
