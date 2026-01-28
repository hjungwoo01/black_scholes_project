#ifndef PAPER_FEED_H
#define PAPER_FEED_H

#include "data_feed_interface.h"
#include <unordered_map>

/** In-memory data feed for paper trading and tests; no network. */
class PaperFeed : public DataFeedInterface {
public:
    OptionalDouble getCurrentPrice(const std::string& symbol) const override;
    void setCurrentPrice(const std::string& symbol, double price) override;
    bool fetchHistoricalPrices(const std::string& symbol, const std::string& start_date, const std::string& end_date) override;
    std::vector<StockPrice> getHistoricalPrices(const std::string& symbol) const override;

private:
    std::unordered_map<std::string, double> current_prices_;
    std::unordered_map<std::string, std::vector<StockPrice>> historical_prices_;
};

#endif
