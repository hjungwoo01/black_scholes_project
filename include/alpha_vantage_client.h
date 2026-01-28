#ifndef ALPHA_VANTAGE_CLIENT_H
#define ALPHA_VANTAGE_CLIENT_H

#include <string>
#include <vector>
#include <unordered_map>
#include <iostream>
#include <thread>
#include <chrono>
#include <nlohmann/json.hpp>
#include "optional_double.h"

class AlphaVantageClient {
public:
    // Constructor with API key
    explicit AlphaVantageClient(const std::string& api_key);

    // Fetch current stock quote
    OptionalDouble getCurrentPrice(const std::string& symbol);

    // Fetch historical daily prices
    std::vector<std::pair<std::string, double>> getHistoricalPrices(
        const std::string& symbol, 
        const std::string& start_date, 
        const std::string& end_date
    );

    // Fetch implied volatility (if available)
    OptionalDouble getImpliedVolatility(const std::string& symbol);

private:
    std::string api_key;
    std::string base_url = "https://www.alphavantage.co/query";

    // Perform HTTP GET request
    std::string performHttpGet(const std::string& url);
};

#endif // ALPHA_VANTAGE_CLIENT_H