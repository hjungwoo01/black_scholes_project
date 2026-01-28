#include "../include/alpha_vantage_client.h"
#include <curl/curl.h>
#include <stdexcept>
#include <sstream>
#include <iomanip>
#include <ctime>
#include <iostream> 

// Callback function for libcurl
static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* output) {
    size_t total_size = size * nmemb;
    output->append((char*)contents, total_size);
    return total_size;
}

// Constructor (empty API key allowed for offline/demo use; API calls will fail until key is set)
AlphaVantageClient::AlphaVantageClient(const std::string& api_key)
    : api_key(api_key) {
    curl_global_init(CURL_GLOBAL_DEFAULT);
}

// Perform HTTP GET request
std::string AlphaVantageClient::performHttpGet(const std::string& url) {
    CURL* curl;
    CURLcode res;
    std::string response_string;

    curl = curl_easy_init();
    if (!curl) {
        throw std::runtime_error("Failed to initialize CURL");
    }

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_string);

    // Perform the request
    res = curl_easy_perform(curl);

    // Check for errors
    if (res != CURLE_OK) {
        curl_easy_cleanup(curl);
        throw std::runtime_error("CURL request failed: " + std::string(curl_easy_strerror(res)));
    }

    // Clean up
    curl_easy_cleanup(curl);

    return response_string;
}

// Get current stock price
std::optional<double> AlphaVantageClient::getCurrentPrice(const std::string& symbol) {
    // Add a small delay to respect API rate limits
    std::this_thread::sleep_for(std::chrono::milliseconds(250));

    // Construct URL for global quote
    std::string url = base_url + 
        "?function=GLOBAL_QUOTE" + 
        "&symbol=" + symbol + 
        "&apikey=" + api_key;

    try {
        std::string response = performHttpGet(url);
        
        // Parse JSON response
        auto json = nlohmann::json::parse(response);

        // Extract price
        if (json.contains("Global Quote") && 
            json["Global Quote"].contains("05. price")) {
            return std::stod(json["Global Quote"]["05. price"].get<std::string>());
        }

        std::cerr << "No price data found for " << symbol << std::endl;
        return std::nullopt;
    }
    catch (const std::exception& e) {
        std::cerr << "Error fetching current price for " << symbol << ": " << e.what() << std::endl;
        return std::nullopt;
    }
}

// Get historical prices
std::vector<std::pair<std::string, double>> AlphaVantageClient::getHistoricalPrices(
    const std::string& symbol, 
    const std::string& start_date, 
    const std::string& end_date
) {
    // Add a small delay to respect API rate limits
    std::this_thread::sleep_for(std::chrono::milliseconds(250));

    // Construct URL for daily adjusted prices
    std::string url = base_url + 
        "?function=TIME_SERIES_DAILY_ADJUSTED" + 
        "&symbol=" + symbol + 
        "&outputsize=full" +  // Full historical data
        "&apikey=" + api_key;

    std::vector<std::pair<std::string, double>> historical_prices;

    try {
        std::string response = performHttpGet(url);
        
        // Parse JSON response
        auto json = nlohmann::json::parse(response);

        // Check if time series data exists
        if (!json.contains("Time Series (Daily)")) {
            std::cerr << "No historical data found for " << symbol << std::endl;
            return historical_prices;
        }

        // Iterate through time series
        for (const auto& [date, price_data] : json["Time Series (Daily)"].items()) {
            // Check date range
            if (date >= start_date && date <= end_date) {
                // Use adjusted close price
                double adjusted_close = std::stod(price_data["5. adjusted close"].get<std::string>());
                historical_prices.emplace_back(date, adjusted_close);
            }
        }

        return historical_prices;
    }
    catch (const std::exception& e) {
        std::cerr << "Error fetching historical prices for " << symbol << ": " << e.what() << std::endl;
        return historical_prices;
    }
}

// Get implied volatility (Note: Alpha Vantage doesn't directly provide this)
std::optional<double> AlphaVantageClient::getImpliedVolatility(const std::string& symbol) {
    // This is a placeholder. Actual implied volatility is not directly available in the free Alpha Vantage API
    return std::nullopt;
}