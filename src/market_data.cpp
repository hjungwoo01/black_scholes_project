#include "market_data.h"
#include "alpha_vantage_feed.h"
#include "paper_feed.h"
#include <algorithm>

MarketDataProvider::MarketDataProvider(std::unique_ptr<DataFeedInterface> feed)
    : feed_(std::move(feed)) {}

MarketDataProvider::MarketDataProvider(const std::string& alpha_vantage_api_key)
    : feed_(std::make_unique<AlphaVantageFeed>(alpha_vantage_api_key)) {}

bool MarketDataProvider::updateCurrentPrice(const std::string& symbol) {
    OptionalDouble price = feed_->getCurrentPrice(symbol);
    if (price) {
        current_prices_cache_[symbol] = *price;
        return true;
    }
    return false;
}

void MarketDataProvider::setCurrentPrice(const std::string& symbol, double price) {
    current_prices_cache_[symbol] = price;
    feed_->setCurrentPrice(symbol, price);
}

OptionalDouble MarketDataProvider::getCurrentPrice(const std::string& symbol) const {
    auto it = current_prices_cache_.find(symbol);
    if (it != current_prices_cache_.end())
        return OptionalDouble(it->second);
    return feed_->getCurrentPrice(symbol);
}

bool MarketDataProvider::fetchHistoricalPrices(
    const std::string& symbol,
    const std::string& start_date,
    const std::string& end_date
) {
    return feed_->fetchHistoricalPrices(symbol, start_date, end_date);
}

std::vector<StockPrice> MarketDataProvider::getHistoricalPrices(const std::string& symbol) const {
    return feed_->getHistoricalPrices(symbol);
}

std::string MarketDataProvider::lastError() const {
    return feed_ ? feed_->lastError() : std::string();
}
