#include "../include/black_scholes_greeks.h"

// Calculate all Greeks for a call option
OptionGreeks BlackScholesGreeks::calculateCallGreeks(
    double spot_price,
    double strike_price,
    double risk_free_rate,
    double volatility,
    double time_to_expiry
) {
    OptionGreeks greeks;
    
    // Calculate d1 and d2 (key components of Black-Scholes formula)
    double d1 = (std::log(spot_price / strike_price) + 
               (risk_free_rate + 0.5 * volatility * volatility) * time_to_expiry) 
              / (volatility * std::sqrt(time_to_expiry));
    
    double d2 = d1 - volatility * std::sqrt(time_to_expiry);
    
    // Calculate delta (1st derivative with respect to underlying price)
    greeks.delta = standardNormalCDF(d1);
    
    // Calculate gamma (2nd derivative with respect to underlying price)
    greeks.gamma = standardNormalPDF(d1) / (spot_price * volatility * std::sqrt(time_to_expiry));
    
    // Calculate theta (1st derivative with respect to time)
    // Note: Theta is usually expressed as the daily decay, so we convert from years to days
    double theta = -spot_price * standardNormalPDF(d1) * volatility / (2 * std::sqrt(time_to_expiry)) 
                   - risk_free_rate * strike_price * std::exp(-risk_free_rate * time_to_expiry) 
                   * standardNormalCDF(d2);
    greeks.theta = theta / 365.0; // Convert from per-year to per-day
    
    // Calculate vega (1st derivative with respect to volatility)
    // Note: Vega is traditionally expressed as change for a 1% change in volatility
    greeks.vega = spot_price * std::sqrt(time_to_expiry) * standardNormalPDF(d1) / 100.0;
    
    // Calculate rho (1st derivative with respect to interest rate)
    // Note: Rho is traditionally expressed as change for a 1% change in interest rate
    greeks.rho = strike_price * time_to_expiry * std::exp(-risk_free_rate * time_to_expiry) 
                * standardNormalCDF(d2) / 100.0;
    
    return greeks;
}

// Calculate all Greeks for a put option
OptionGreeks BlackScholesGreeks::calculatePutGreeks(
    double spot_price,
    double strike_price,
    double risk_free_rate,
    double volatility,
    double time_to_expiry
) {
    OptionGreeks greeks;
    
    // Calculate d1 and d2 (key components of Black-Scholes formula)
    double d1 = (std::log(spot_price / strike_price) + 
               (risk_free_rate + 0.5 * volatility * volatility) * time_to_expiry) 
              / (volatility * std::sqrt(time_to_expiry));
    
    double d2 = d1 - volatility * std::sqrt(time_to_expiry);
    
    // Calculate delta (1st derivative with respect to underlying price)
    greeks.delta = standardNormalCDF(d1) - 1;
    
    // Calculate gamma (2nd derivative with respect to underlying price)
    // Note: Gamma is the same for both calls and puts
    greeks.gamma = standardNormalPDF(d1) / (spot_price * volatility * std::sqrt(time_to_expiry));
    
    // Calculate theta (1st derivative with respect to time)
    // Note: Theta is usually expressed as the daily decay, so we convert from years to days
    double theta = -spot_price * standardNormalPDF(d1) * volatility / (2 * std::sqrt(time_to_expiry)) 
                   + risk_free_rate * strike_price * std::exp(-risk_free_rate * time_to_expiry) 
                   * standardNormalCDF(-d2);
    greeks.theta = theta / 365.0; // Convert from per-year to per-day
    
    // Calculate vega (1st derivative with respect to volatility)
    // Note: Vega is traditionally expressed as change for a 1% change in volatility
    // Note: Vega is the same for both calls and puts
    greeks.vega = spot_price * std::sqrt(time_to_expiry) * standardNormalPDF(d1) / 100.0;
    
    // Calculate rho (1st derivative with respect to interest rate)
    // Note: Rho is traditionally expressed as change for a 1% change in interest rate
    greeks.rho = -strike_price * time_to_expiry * std::exp(-risk_free_rate * time_to_expiry) 
                * standardNormalCDF(-d2) / 100.0;
    
    return greeks;
}