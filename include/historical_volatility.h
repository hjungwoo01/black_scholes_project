#ifndef HISTORICAL_VOLATILITY_H
#define HISTORICAL_VOLATILITY_H

#include <vector>
#include <string>
#include <cmath>
#include <algorithm>
#include <stdexcept>
#include "../include/market_data.h"

class HistoricalVolatility {
public:
    // Calculate historical volatility from a series of prices
    // Returns annualized volatility (as a decimal, not percentage)
    static double calculateFromPrices(const std::vector<double>& prices) {
        if (prices.size() < 2) {
            throw std::invalid_argument("Need at least two price points to calculate volatility");
        }

        // Calculate log returns: ln(price_t / price_{t-1})
        std::vector<double> log_returns;
        log_returns.reserve(prices.size() - 1);
        
        for (size_t i = 1; i < prices.size(); ++i) {
            if (prices[i] <= 0 || prices[i-1] <= 0) {
                throw std::invalid_argument("Prices must be positive for log return calculation");
            }
            log_returns.push_back(std::log(prices[i] / prices[i-1]));
        }
        
        // Calculate mean of log returns
        double sum = 0.0;
        for (double ret : log_returns) {
            sum += ret;
        }
        double mean = sum / log_returns.size();
        
        // Calculate variance of log returns
        double variance = 0.0;
        for (double ret : log_returns) {
            double diff = ret - mean;
            variance += diff * diff;
        }
        variance /= (log_returns.size() - 1);  // Use sample variance (n-1 denominator)
        
        // Standard deviation of log returns
        double volatility = std::sqrt(variance);
        
        // Annualize volatility (assuming daily returns)
        return volatility * std::sqrt(252.0);  // 252 trading days in a year
    }
    
    // Calculate historical volatility from StockPrice objects
    static double calculateFromStockPrices(const std::vector<StockPrice>& stock_prices) {
        if (stock_prices.empty()) {
            throw std::invalid_argument("Empty price vector");
        }
        
        // Extract prices from StockPrice objects
        std::vector<double> prices;
        prices.reserve(stock_prices.size());
        
        for (const auto& sp : stock_prices) {
            prices.push_back(sp.price);
        }
        
        // Ensure prices are in chronological order (oldest to newest)
        std::reverse(prices.begin(), prices.end());
        
        return calculateFromPrices(prices);
    }
    
    // Calculate historical volatility using a specific timeframe (in trading days)
    static double calculateWithWindow(
        const std::vector<double>& prices, 
        int window_days = 30  // Default 30-day historical volatility
    ) {
        if (prices.size() < window_days + 1) {
            throw std::invalid_argument("Not enough price data for the specified window");
        }
        
        // Use only the most recent window_days prices
        std::vector<double> recent_prices(
            prices.end() - window_days - 1,
            prices.end()
        );
        
        return calculateFromPrices(recent_prices);
    }
};

#endif // HISTORICAL_VOLATILITY_H