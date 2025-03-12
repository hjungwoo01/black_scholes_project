#include "../include/black_scholes.h"
#include <stdexcept>
#include <cmath>

// Standard Normal Cumulative Distribution Function
double BlackScholes::standardNormalCDF(double x) {
    return 0.5 * (1.0 + std::erf(x / std::sqrt(2.0)));
}

// Standard Normal Probability Density Function
double BlackScholes::standardNormalPDF(double x) {
    return (1.0 / std::sqrt(2.0 * M_PI)) * std::exp(-0.5 * x * x);
}

// Calculate Call Option Price
double BlackScholes::calculateCallPrice(
    double spot_price, 
    double strike_price, 
    double risk_free_rate, 
    double volatility, 
    double time_to_expiry
) {
    // Validate inputs
    if (spot_price <= 0 || strike_price <= 0 || time_to_expiry <= 0 || volatility <= 0) {
        throw std::invalid_argument("Invalid input parameters for Black-Scholes model");
    }

    // Calculate d1 and d2
    double d1 = (std::log(spot_price / strike_price) + 
                 (risk_free_rate + 0.5 * volatility * volatility) * time_to_expiry) 
                / (volatility * std::sqrt(time_to_expiry));
    
    double d2 = d1 - volatility * std::sqrt(time_to_expiry);

    // Calculate call option price
    double call_price = spot_price * standardNormalCDF(d1) - 
                        strike_price * std::exp(-risk_free_rate * time_to_expiry) * standardNormalCDF(d2);

    return call_price;
}

// Calculate Put Option Price
double BlackScholes::calculatePutPrice(
    double spot_price, 
    double strike_price, 
    double risk_free_rate, 
    double volatility, 
    double time_to_expiry
) {
    // Validate inputs
    if (spot_price <= 0 || strike_price <= 0 || time_to_expiry <= 0 || volatility <= 0) {
        throw std::invalid_argument("Invalid input parameters for Black-Scholes model");
    }

    // Calculate d1 and d2
    double d1 = (std::log(spot_price / strike_price) + 
                 (risk_free_rate + 0.5 * volatility * volatility) * time_to_expiry) 
                / (volatility * std::sqrt(time_to_expiry));
    
    double d2 = d1 - volatility * std::sqrt(time_to_expiry);

    // Calculate put option price
    double put_price = strike_price * std::exp(-risk_free_rate * time_to_expiry) * standardNormalCDF(-d2) - 
                       spot_price * standardNormalCDF(-d1);

    return put_price;
}

// Calculate Implied Volatility using Newton-Raphson method
double BlackScholes::calculateImpliedVolatility(
    double market_price,
    double spot_price,
    double strike_price,
    double risk_free_rate,
    double time_to_expiry,
    OptionType option_type
) {
    // Initial guess for volatility
    double volatility = 0.3;  // 30% volatility
    const int max_iterations = 100;
    const double tolerance = 1e-5;

    for (int i = 0; i < max_iterations; ++i) {
        double model_price, vega;
        
        // Calculate model price and vega based on option type
        if (option_type == CALL) {
            model_price = calculateCallPrice(spot_price, strike_price, risk_free_rate, volatility, time_to_expiry);
            
            // Calculate d1 for vega
            double d1 = (std::log(spot_price / strike_price) + 
                         (risk_free_rate + 0.5 * volatility * volatility) * time_to_expiry) 
                        / (volatility * std::sqrt(time_to_expiry));
            
            // Vega calculation
            vega = spot_price * standardNormalPDF(d1) * std::sqrt(time_to_expiry);
        } else {
            model_price = calculatePutPrice(spot_price, strike_price, risk_free_rate, volatility, time_to_expiry);
            
            // Calculate d1 for vega
            double d1 = (std::log(spot_price / strike_price) + 
                         (risk_free_rate + 0.5 * volatility * volatility) * time_to_expiry) 
                        / (volatility * std::sqrt(time_to_expiry));
            
            // Vega calculation
            vega = spot_price * standardNormalPDF(d1) * std::sqrt(time_to_expiry);
        }

        // Newton-Raphson method
        double price_diff = model_price - market_price;
        
        // Check convergence
        if (std::abs(price_diff) < tolerance) {
            return volatility;
        }

        // Update volatility
        volatility -= price_diff / vega;

        // Prevent negative volatility
        volatility = std::max(0.0001, volatility);
    }

    // If max iterations reached, return last calculated volatility
    return volatility;
}