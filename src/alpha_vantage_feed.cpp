#include "alpha_vantage_feed.h"

AlphaVantageFeed::AlphaVantageFeed(const std::string& api_key) : client_(api_key) {
    last_error_.clear();
}

OptionalDouble AlphaVantageFeed::getCurrentPrice(const std::string& symbol) const {
    last_error_.clear();
    try {
        return client_.getCurrentPrice(symbol);
    } catch (const std::exception& e) {
        last_error_ = e.what();
        return OptionalDouble();
    }
}

bool AlphaVantageFeed::fetchHistoricalPrices(const std::string& symbol, const std::string& start_date, const std::string& end_date) {
    last_error_.clear();
    try {
        auto pairs = client_.getHistoricalPrices(symbol, start_date, end_date);
        if (pairs.empty()) return false;
        std::vector<StockPrice> out;
        for (const auto& p : pairs)
            out.push_back({p.second, p.first});
        historical_cache_[symbol] = std::move(out);
        return true;
    } catch (const std::exception& e) {
        last_error_ = e.what();
        return false;
    }
}

std::vector<StockPrice> AlphaVantageFeed::getHistoricalPrices(const std::string& symbol) const {
    auto it = historical_cache_.find(symbol);
    if (it != historical_cache_.end())
        return it->second;
    return {};
}
