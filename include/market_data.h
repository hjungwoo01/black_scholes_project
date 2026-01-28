#ifndef MARKET_DATA_H
#define MARKET_DATA_H

#include "data_feed_interface.h"
#include "optional_double.h"
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

class MarketDataProvider {
public:
    /** Take ownership of a data feed (e.g. AlphaVantageFeed or PaperFeed). */
    explicit MarketDataProvider(std::unique_ptr<DataFeedInterface> feed);

    /** Backwards-compatible: build an AlphaVantageFeed from API key. */
    explicit MarketDataProvider(const std::string& alpha_vantage_api_key);

    MarketDataProvider(const MarketDataProvider&) = delete;
    MarketDataProvider& operator=(const MarketDataProvider&) = delete;

    bool updateCurrentPrice(const std::string& symbol);
    void setCurrentPrice(const std::string& symbol, double price);
    OptionalDouble getCurrentPrice(const std::string& symbol) const;

    bool fetchHistoricalPrices(
        const std::string& symbol,
        const std::string& start_date,
        const std::string& end_date
    );

    std::vector<StockPrice> getHistoricalPrices(const std::string& symbol) const;

    /** Last error from the underlying feed, if any. */
    std::string lastError() const;

private:
    std::unique_ptr<DataFeedInterface> feed_;
    std::unordered_map<std::string, double> current_prices_cache_;
};

#endif
