#ifndef DATA_FEED_INTERFACE_H
#define DATA_FEED_INTERFACE_H

#include "optional_double.h"
#include <string>
#include <vector>

struct StockPrice {
    double price{0.0};
    std::string timestamp;
};

/** Abstract base for market data feeds (Alpha Vantage, Paper, Alpaca, etc.). */
class DataFeedInterface {
public:
    virtual ~DataFeedInterface() = default;

    virtual OptionalDouble getCurrentPrice(const std::string& symbol) const = 0;

    /** Optional: push a price (no-op for live API feeds; PaperFeed overrides). */
    virtual void setCurrentPrice(const std::string& symbol, double price) { (void)symbol; (void)price; }

    virtual bool fetchHistoricalPrices(
        const std::string& symbol,
        const std::string& start_date,
        const std::string& end_date
    ) = 0;

    virtual std::vector<StockPrice> getHistoricalPrices(const std::string& symbol) const = 0;

    /** Last error message (empty if none). Optional for implementers. */
    virtual std::string lastError() const { return {}; }

    /** True if the last quote for symbol is older than max_age_seconds (if feed supports timestamps). */
    virtual bool isStaleQuote(const std::string& symbol, int max_age_seconds) const { (void)symbol; (void)max_age_seconds; return false; }
};

#endif
