#include "../include/market_data.h"
#include <stdexcept>
#include <iomanip>
#include <sstream>
#include <ctime>

MarketDataProvider::MarketDataProvider(const std::string& alpha_vantage_api_key)
    : api_client(alpha_vantage_api_key) {}

bool MarketDataProvider::updateCurrentPrice(const std::string& symbol) {
    auto price = api_client.getCurrentPrice(symbol);
    if (price) {
        current_prices[symbol] = *price;
        return true;
    }
    return false;
}

void MarketDataProvider::setCurrentPrice(const std::string& symbol, double price) {
    current_prices[symbol] = price;
}

OptionalDouble MarketDataProvider::getCurrentPrice(const std::string& symbol) const {
    auto it = current_prices.find(symbol);
    if (it != current_prices.end()) {
        return OptionalDouble(it->second);
    }
    return OptionalDouble();
}

bool MarketDataProvider::fetchHistoricalPrices(
    const std::string& symbol, 
    const std::string& start_date, 
    const std::string& end_date
) {
    // Fetch historical prices from Alpha Vantage
    auto prices = api_client.getHistoricalPrices(symbol, start_date, end_date);
    
    if (prices.empty()) {
        return false;
    }
    
    // Convert and store historical prices
    std::vector<StockPrice> stock_prices;
    for (const auto& [date, price] : prices) {
        stock_prices.push_back({price, date});
    }
    
    historical_prices[symbol] = stock_prices;
    return true;
}

std::vector<StockPrice> MarketDataProvider::getHistoricalPrices(const std::string& symbol) const {
    auto it = historical_prices.find(symbol);
    if (it != historical_prices.end()) {
        return it->second;
    }
    return {};
}