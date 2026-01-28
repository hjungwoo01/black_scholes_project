#include "paper_feed.h"

OptionalDouble PaperFeed::getCurrentPrice(const std::string& symbol) const {
    auto it = current_prices_.find(symbol);
    if (it != current_prices_.end())
        return OptionalDouble(it->second);
    return OptionalDouble();
}

void PaperFeed::setCurrentPrice(const std::string& symbol, double price) {
    current_prices_[symbol] = price;
}

bool PaperFeed::fetchHistoricalPrices(const std::string& symbol, const std::string&, const std::string&) {
    (void)symbol;
    return false;
}

std::vector<StockPrice> PaperFeed::getHistoricalPrices(const std::string& symbol) const {
    auto it = historical_prices_.find(symbol);
    if (it != historical_prices_.end())
        return it->second;
    return {};
}
