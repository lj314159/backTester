#pragma once

#include <memory>
#include <string>
#include <nlohmann/json.hpp>

#include "Strategy_I.hpp"

std::unique_ptr<Strategy_I>
createStrategy(const std::string &symbol,
               const nlohmann::json &stratCfg);
