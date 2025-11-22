#include "AlphaVantageFeed.h"
#include "MarketDataFeed_I.h"

#include <curl/curl.h>
#include <nlohmann/json.hpp>

#include <algorithm>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

using nlohmann::json;

namespace
{

size_t writeCallback(char *ptr, size_t size, size_t nmemb, void *userdata)
{
  std::size_t totalSize = size * nmemb;
  auto *buffer = static_cast<std::string *>(userdata);
  buffer->append(ptr, totalSize);
  return totalSize;
}

// Helper to perform a simple HTTP GET using libcurl.
std::string httpGet(const std::string &url)
{
  CURL *curl = curl_easy_init();
  if(curl == nullptr)
  {
    throw std::runtime_error("Failed to initialize CURL");
  }

  std::string response;
  curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
  curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

  CURLcode res = curl_easy_perform(curl);
  if(res != CURLE_OK)
  {
    std::string msg = "CURL error: ";
    msg += curl_easy_strerror(res);
    curl_easy_cleanup(curl);
    throw std::runtime_error(msg);
  }

  long httpCode = 0;
  curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);
  curl_easy_cleanup(curl);

  if(httpCode != 200)
  {
    std::ostringstream oss;
    oss << "HTTP error code " << httpCode;
    throw std::runtime_error(oss.str());
  }

  return response;
}

} // namespace

AlphaVantageFeed::AlphaVantageFeed(std::vector<Candle> candles)
  : candles_(std::move(candles)),
    index_(0)
{
}

bool AlphaVantageFeed::hasNext() const
{
  if(index_ < candles_.size())
  {
    return true;
  }
  return false;
}

const Candle &AlphaVantageFeed::next()
{
  if(!hasNext())
  {
    throw std::out_of_range("AlphaVantageFeed::next called with no more data");
  }
  return candles_[index_++];
}

std::size_t AlphaVantageFeed::currentIndex() const
{
  return index_;
}

std::unique_ptr<MarketDataFeed_I>
makeAlphaVantageFeed(const std::string &apiKey,
                     const std::string &symbol,
                     int lookbackBars)
{
  // ------------------------------------------------------------------
  // Build URL:
  //   TIME_SERIES_DAILY, full history, then trim to lookbackBars.
  // ------------------------------------------------------------------
  std::ostringstream url;
  url << "https://www.alphavantage.co/query?function=TIME_SERIES_DAILY"
      << "&symbol=" << symbol
      << "&outputsize=full"
      << "&apikey=" << apiKey;

  std::string raw = httpGet(url.str());

  // ------------------------------------------------------------------
  // Parse JSON.
  // ------------------------------------------------------------------
  json j;
  try
  {
    j = json::parse(raw);
  }
  catch(const std::exception &ex)
  {
    std::string msg = "Failed to parse Alpha Vantage JSON: ";
    msg += ex.what();
    throw std::runtime_error(msg);
  }

  const char *seriesKey = "Time Series (Daily)";

  if(!j.contains(seriesKey))
  {
    std::ostringstream oss;
    oss << "Alpha Vantage response missing 'Time Series (Daily)'. "
        << "Raw response (truncated): "
        << raw.substr(0, 400);
    throw std::runtime_error(oss.str());
  }

  const json &series = j.at(seriesKey);

  if(!series.is_object())
  {
    throw std::runtime_error("Alpha Vantage 'Time Series (Daily)' is not an object");
  }

  // ------------------------------------------------------------------
  // Collect all dates, sort, and apply lookback.
  // ------------------------------------------------------------------
  std::vector<std::string> dates;
  dates.reserve(series.size());
  for(auto it = series.begin(); it != series.end(); ++it)
  {
    dates.push_back(it.key());
  }

  // Sort ascending by date string (YYYY-MM-DD) so earliest first.
  std::sort(dates.begin(), dates.end());

  if(lookbackBars > 0)
  {
    if(static_cast<int>(dates.size()) > lookbackBars)
    {
      // Keep only the most recent lookbackBars dates.
      dates.erase(dates.begin(),
                  dates.end() - static_cast<std::size_t>(lookbackBars));
    }
  }

  std::vector<Candle> candles;
  candles.reserve(dates.size());

  for(const std::string &date : dates)
  {
    const json &bar = series.at(date);

    Candle c;
    c.timestamp = date;
    c.open = std::stod(bar.at("1. open").get<std::string>());
    c.high = std::stod(bar.at("2. high").get<std::string>());
    c.low = std::stod(bar.at("3. low").get<std::string>());
    c.close = std::stod(bar.at("4. close").get<std::string>());
    c.volume = std::stod(bar.at("5. volume").get<std::string>());

    candles.push_back(c);
  }

  if(candles.empty())
  {
    throw std::runtime_error("Alpha Vantage returned no candles for symbol " + symbol);
  }

  return std::make_unique<AlphaVantageFeed>(std::move(candles));
}
