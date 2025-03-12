#ifndef MARKET_DATA_H
#define MARKET_DATA_H

#include <string>
#include <unordered_map>
#include <vector>
#include <ctime>
#include <optional>
#include "../include/alpha_vantage_client.h"

struct StockPrice {
    double price;
    std::string timestamp;
};

class MarketDataProvider {
private:
    AlphaVantageClient api_client;
    std::unordered_map<std::string, std::vector<StockPrice>> historical_prices;
    std::unordered_map<std::string, double> current_prices;

public:
    // Constructor with API key
    MarketDataProvider(const std::string& alpha_vantage_api_key);

    // Fetch and update current price
    bool updateCurrentPrice(const std::string& symbol);
    
    // Get current price
    std::optional<double> getCurrentPrice(const std::string& symbol) const;
    
    // Fetch historical prices
    bool fetchHistoricalPrices(
        const std::string& symbol, 
        const std::string& start_date, 
        const std::string& end_date
    );
    
    // Get historical prices
    std::vector<StockPrice> getHistoricalPrices(const std::string& symbol) const;
};

#endif // MARKET_DATA_H