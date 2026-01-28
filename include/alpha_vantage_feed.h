#ifndef ALPHA_VANTAGE_FEED_H
#define ALPHA_VANTAGE_FEED_H

#include "data_feed_interface.h"
#include "alpha_vantage_client.h"
#include <string>
#include <unordered_map>

/** DataFeedInterface implementation using Alpha Vantage API. */
class AlphaVantageFeed : public DataFeedInterface {
public:
    explicit AlphaVantageFeed(const std::string& api_key);

    OptionalDouble getCurrentPrice(const std::string& symbol) const override;
    bool fetchHistoricalPrices(const std::string& symbol, const std::string& start_date, const std::string& end_date) override;
    std::vector<StockPrice> getHistoricalPrices(const std::string& symbol) const override;
    std::string lastError() const override { return last_error_; }

private:
    mutable AlphaVantageClient client_;
    mutable std::string last_error_;
    std::unordered_map<std::string, std::vector<StockPrice>> historical_cache_;
};

#endif
