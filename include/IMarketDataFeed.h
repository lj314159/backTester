#pragma once

#include "Types.h"
#include <cstddef>

class IMarketDataFeed {
public:
    virtual ~IMarketDataFeed() = default;

    virtual bool hasNext() const = 0;
    virtual const Candle& next() = 0;
    virtual std::size_t currentIndex() const = 0;
};
