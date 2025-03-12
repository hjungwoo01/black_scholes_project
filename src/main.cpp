#include "../include/option.h"
#include "../include/black_scholes.h"
#include "../include/black_scholes_greeks.h"
#include "../include/market_data.h"
#include <iostream>
#include <ctime>
#include <iomanip>
#include <vector>
#include <chrono>
#include <thread>
#include <fstream>
#include <sstream>
#include <cstdlib>

// Function to find the next available Friday
std::time_t getNextFriday(int weeks_ahead) {
    std::time_t now = std::time(nullptr);
    std::tm time_info = *std::localtime(&now);
    
    // Find how many days until the next Friday
    int days_until_friday = (5 - time_info.tm_wday + 7) % 7;
    if (days_until_friday == 0) {
        days_until_friday = 7;
    }
    
    // Move forward in weeks
    days_until_friday += weeks_ahead * 7;
    
    // Calculate the next Friday's date
    time_info.tm_mday += days_until_friday;
    std::mktime(&time_info);
    return std::mktime(&time_info);
}

// Function to load a specific key from the .env file (unchanged)
std::string loadEnvValue(const std::string& key, const std::string& filename) {
    std::ifstream file(filename);
    std::string line, var, value;
    if (file.is_open()) {
        while (std::getline(file, line)) {
            std::istringstream iss(line);
            if (std::getline(iss, var, '=') && std::getline(iss, value)) {
                if (var == key) {
                    return value;
                }
            }
        }
        file.close();
    }
    return "";
}

// Function to print option details with Greeks
void printOptionDetailsWithGreeks(
    const std::string& symbol,
    double current_price,
    double strike_price,
    std::time_t expiry,
    double time_to_expiry,
    double risk_free_rate,
    double volatility,
    int dte
) {
    // Calculate option prices
    double call_price = BlackScholes::calculateCallPrice(
        current_price, strike_price, risk_free_rate, volatility, time_to_expiry
    );
    
    double put_price = BlackScholes::calculatePutPrice(
        current_price, strike_price, risk_free_rate, volatility, time_to_expiry
    );
    
    // Calculate Greeks
    OptionGreeks call_greeks = BlackScholesGreeks::calculateCallGreeks(
        current_price, strike_price, risk_free_rate, volatility, time_to_expiry
    );
    
    OptionGreeks put_greeks = BlackScholesGreeks::calculatePutGreeks(
        current_price, strike_price, risk_free_rate, volatility, time_to_expiry
    );
    
    // Print basic option information
    std::cout << std::fixed << std::setprecision(2)
              << std::left << std::setw(10) << strike_price << "| "
              << std::setw(7) << std::put_time(std::localtime(&expiry), "%Y-%m-%d") << "     |"
              << std::setw(10) << dte << "|"
              << std::setw(10) << call_price << "      |"
              << std::setw(10) << put_price
              << "\n";
    
    // Print Greeks for call option
    std::cout << "  Call Greeks: "
              << "Δ=" << std::setprecision(3) << call_greeks.delta << " "
              << "Γ=" << call_greeks.gamma << " "
              << "Θ=" << call_greeks.theta << " "
              << "ν=" << call_greeks.vega << " "
              << "ρ=" << call_greeks.rho
              << "\n";
    
    // Print Greeks for put option
    std::cout << "  Put Greeks:  "
              << "Δ=" << std::setprecision(3) << put_greeks.delta << " "
              << "Γ=" << put_greeks.gamma << " "
              << "Θ=" << put_greeks.theta << " "
              << "ν=" << put_greeks.vega << " "
              << "ρ=" << put_greeks.rho
              << "\n";
    
    std::cout << "--------------------------------------------------------------------------------\n";
}

int main() {
    std::string alpha_vantage_api_key = loadEnvValue("ALPHA_VANTAGE_API_KEY", ".env");
    
    // Constants for retry logic
    const int MAX_RETRIES = 3;
    const int RETRY_DELAY_SECONDS = 3; // Increased delay between retries
    const int API_CALL_DELAY_SECONDS = 1; // Delay between API calls for different symbols
    
    try {
        // Create market data provider
        MarketDataProvider market_data(alpha_vantage_api_key);
        
        // List of stock symbols
        std::vector<std::string> stock_symbols = {"TSLA", "NVDA", "AMZN", "AAPL", "GOOG"};
        
        // Risk-free rate and assumed volatility
        double risk_free_rate = 0.02; // 2%
        double volatility = 0.3; // 30%
        
        // Define strike price multipliers (relative to current price)
        std::vector<double> strike_multipliers = {0.9, 1.0, 1.1, 1.2, 1.3};
        
        // Get expiration dates (next 3 Fridays)
        std::vector<std::time_t> expirations = {getNextFriday(0), getNextFriday(1), getNextFriday(4)};
        
        for (const auto& symbol : stock_symbols) {
            bool success = false;
            
            // Try up to MAX_RETRIES times
            for (int retry = 0; retry < MAX_RETRIES && !success; retry++) {
                if (retry > 0) {
                    std::cout << "Retrying " << symbol << " (attempt " << retry+1 << "/" 
                              << MAX_RETRIES << "). Waiting " << RETRY_DELAY_SECONDS 
                              << " seconds..." << std::endl;
                    
                    std::this_thread::sleep_for(std::chrono::seconds(RETRY_DELAY_SECONDS));
                }
                
                success = market_data.updateCurrentPrice(symbol);
            }
            
            if (!success) {
                std::cerr << "Failed to fetch current price for " << symbol 
                          << " after " << MAX_RETRIES << " attempts" << std::endl;
                continue;
            }
            
            auto current_price = market_data.getCurrentPrice(symbol);
            if (!current_price) {
                std::cerr << "No price data available for " << symbol << std::endl;
                continue;
            }
            
            std::cout << "\n================================================================================\n";
            std::cout << "Option Fair Value for " << symbol << " (Current Price: " << *current_price << ")\n";
            std::cout << "================================================================================\n";
            std::cout << std::left 
                      << std::setw(10) << "Strike" << "| "
                      << std::setw(15) << "Expiration" << "| "
                      << std::setw(10) << "DTE" << "| "
                      << std::setw(15) << "Call Price" << "| "
                      << std::setw(15) << "Put Price"
                      << "\n";
            std::cout << "--------------------------------------------------------------------------------\n";
            
            for (double multiplier : strike_multipliers) {
                double strike_price = *current_price * multiplier;
                
                for (std::time_t expiry : expirations) {
                    std::time_t now = std::time(nullptr);
                    double time_to_expiry = difftime(expiry, now) / (60 * 60 * 24 * 365); // Convert to years
                    int dte = difftime(expiry, now) / (60 * 60 * 24); // Days to expiration
                    
                    printOptionDetailsWithGreeks(
                        symbol, *current_price, strike_price, expiry, 
                        time_to_expiry, risk_free_rate, volatility, dte
                    );
                }
            }
            std::cout << "================================================================================\n";
            
            // Add delay between processing different symbols to respect API rate limits
            if (&symbol != &stock_symbols.back()) {
                std::cout << "Waiting " << API_CALL_DELAY_SECONDS 
                          << " seconds before processing next symbol..." << std::endl;
                std::this_thread::sleep_for(std::chrono::seconds(API_CALL_DELAY_SECONDS));
            }
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}