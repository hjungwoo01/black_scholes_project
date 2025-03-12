#ifndef MONTE_CARLO_H
#define MONTE_CARLO_H

#include <random>
#include <cmath>
#include <vector>
#include "../include/option.h"

class MonteCarloOptionPricer {
public:
    // Price a call option using Monte Carlo simulation
    static double priceCallOption(
        double spot_price,
        double strike_price,
        double risk_free_rate,
        double volatility,
        double time_to_expiry,
        int num_simulations = 10000
    ) {
        double dt = time_to_expiry / 252.0;  // Assuming daily steps over the time to expiry
        double sqrt_dt = std::sqrt(dt);
        
        // Set up random number generator
        std::random_device rd;
        std::mt19937 gen(rd());
        std::normal_distribution<> normal_dist(0.0, 1.0);
        
        double sum_payoffs = 0.0;
        
        for (int i = 0; i < num_simulations; ++i) {
            // Simulate stock price path
            double stock_price = spot_price;
            
            for (int t = 0; t < 252 * time_to_expiry; ++t) {
                double z = normal_dist(gen);
                // Geometric Brownian Motion for stock price
                stock_price *= exp((risk_free_rate - 0.5 * volatility * volatility) * dt + 
                                  volatility * sqrt_dt * z);
            }
            
            // Calculate call option payoff at expiry
            double payoff = std::max(stock_price - strike_price, 0.0);
            sum_payoffs += payoff;
        }
        
        // Average payoff discounted to present value
        double option_price = std::exp(-risk_free_rate * time_to_expiry) * 
                             (sum_payoffs / num_simulations);
        
        return option_price;
    }
    
    // Price a put option using Monte Carlo simulation
    static double pricePutOption(
        double spot_price,
        double strike_price,
        double risk_free_rate,
        double volatility,
        double time_to_expiry,
        int num_simulations = 10000
    ) {
        double dt = time_to_expiry / 252.0;  // Assuming daily steps over the time to expiry
        double sqrt_dt = std::sqrt(dt);
        
        // Set up random number generator
        std::random_device rd;
        std::mt19937 gen(rd());
        std::normal_distribution<> normal_dist(0.0, 1.0);
        
        double sum_payoffs = 0.0;
        
        for (int i = 0; i < num_simulations; ++i) {
            // Simulate stock price path
            double stock_price = spot_price;
            
            for (int t = 0; t < 252 * time_to_expiry; ++t) {
                double z = normal_dist(gen);
                // Geometric Brownian Motion for stock price
                stock_price *= exp((risk_free_rate - 0.5 * volatility * volatility) * dt + 
                                  volatility * sqrt_dt * z);
            }
            
            // Calculate put option payoff at expiry
            double payoff = std::max(strike_price - stock_price, 0.0);
            sum_payoffs += payoff;
        }
        
        // Average payoff discounted to present value
        double option_price = std::exp(-risk_free_rate * time_to_expiry) * 
                             (sum_payoffs / num_simulations);
        
        return option_price;
    }
    
    // Monte Carlo pricing with price path storage (for visualization or analysis)
    static std::vector<std::vector<double>> simulatePricePaths(
        double spot_price,
        double risk_free_rate,
        double volatility,
        double time_to_expiry,
        int num_paths = 100,
        int steps_per_path = 252  // Default to daily steps for a year
    ) {
        double dt = time_to_expiry / steps_per_path;
        double sqrt_dt = std::sqrt(dt);
        
        // Set up random number generator
        std::random_device rd;
        std::mt19937 gen(rd());
        std::normal_distribution<> normal_dist(0.0, 1.0);
        
        // Initialize the price paths matrix
        std::vector<std::vector<double>> price_paths(num_paths, 
                                                    std::vector<double>(steps_per_path + 1, 0.0));
        
        // Set initial price for all paths
        for (int i = 0; i < num_paths; ++i) {
            price_paths[i][0] = spot_price;
        }
        
        // Simulate price paths
        for (int i = 0; i < num_paths; ++i) {
            for (int t = 1; t <= steps_per_path; ++t) {
                double z = normal_dist(gen);
                price_paths[i][t] = price_paths[i][t-1] * exp(
                    (risk_free_rate - 0.5 * volatility * volatility) * dt + 
                    volatility * sqrt_dt * z
                );
            }
        }
        
        return price_paths;
    }
};

#endif // MONTE_CARLO_H