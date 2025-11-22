#include "AlphaVantageFeed.h"

#include <curl/curl.h>
#include <nlohmann/json.hpp>

#include <algorithm>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

using nlohmann::json;

namespace
{
size_t writeCallback(char *ptr,
                     size_t size,
                     size_t nmemb,
                     void *userdata)
{
  std::size_t total = size * nmemb;
  std::string *buffer = static_cast<std::string *>(userdata);
  buffer->append(ptr, total);
  return total;
}
} // namespace

AlphaVantageFeed::AlphaVantageFeed(const std::string &apiKey,
                                   const std::string &symbol)
{
  loadDailySeries(apiKey, symbol);
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
  if(index_ >= candles_.size())
  {
    throw std::out_of_range("AlphaVantageFeed::next: no more data");
  }
  return candles_.at(index_++);
}

std::size_t AlphaVantageFeed::currentIndex() const
{
  if(index_ == 0)
  {
    return 0;
  }
  return index_ - 1;
}

void AlphaVantageFeed::loadDailySeries(const std::string &apiKey,
                                       const std::string &symbol)
{
  CURL *curl = curl_easy_init();
  if(curl == nullptr)
  {
    throw std::runtime_error("Failed to initialize CURL");
  }

  std::string buffer;

  // Use FREE daily endpoint (not adjusted).
  std::string url =
    "https://www.alphavantage.co/query?function=TIME_SERIES_DAILY"
    "&outputsize=compact&symbol=" +
    symbol + "&apikey=" + apiKey;

  curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);

  CURLcode res = curl_easy_perform(curl);
  if(res != CURLE_OK)
  {
    curl_easy_cleanup(curl);
    throw std::runtime_error(std::string("curl_easy_perform failed: ") +
                             curl_easy_strerror(res));
  }

  long httpCode = 0;
  curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);
  curl_easy_cleanup(curl);

  if(httpCode != 200)
  {
    throw std::runtime_error("HTTP error from Alpha Vantage, code " +
                             std::to_string(httpCode));
  }

  json j = json::parse(buffer, nullptr, true, true);

  // Check for rate limit / error messages first.
  if(j.contains("Note"))
  {
    std::string note = j.at("Note").get<std::string>();
    throw std::runtime_error(std::string("Alpha Vantage Note: ") + note);
  }

  if(j.contains("Error Message"))
  {
    std::string msg = j.at("Error Message").get<std::string>();
    throw std::runtime_error(std::string("Alpha Vantage Error Message: ") + msg);
  }

  if(!j.contains("Time Series (Daily)"))
  {
    throw std::runtime_error(
      "Alpha Vantage response missing 'Time Series (Daily)'. "
      "Raw response (truncated): " +
      buffer.substr(0, 200));
  }

  const auto &ts = j.at("Time Series (Daily)");

  std::vector<Candle> temp;

  for(auto it = ts.begin(); it != ts.end(); ++it)
  {
    const std::string date = it.key();
    const json &bar = it.value();

    Candle c;
    c.timestamp = date;
    c.open   = std::stod(bar.at("1. open").get<std::string>());
    c.high   = std::stod(bar.at("2. high").get<std::string>());
    c.low    = std::stod(bar.at("3. low").get<std::string>());
    c.close  = std::stod(bar.at("4. close").get<std::string>());

    // For TIME_SERIES_DAILY, volume is "5. volume" (NOT "6. volume").
    c.volume = std::stod(bar.at("5. volume").get<std::string>());

    temp.push_back(c);
  }

  std::sort(temp.begin(), temp.end(),
            [](const Candle &a, const Candle &b)
            {
              return a.timestamp < b.timestamp;
            });

  candles_ = std::move(temp);

  if(candles_.empty())
  {
    throw std::runtime_error("No candles loaded from Alpha Vantage.");
  }
}

std::unique_ptr<IMarketDataFeed>
makeAlphaVantageFeed(const std::string &apiKey,
                     const std::string &symbol)
{
  return std::make_unique<AlphaVantageFeed>(apiKey, symbol);
}
