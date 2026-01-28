#include "../include/black_scholes_greeks.h"

namespace {
constexpr double MIN_VOLATILITY = 1e-10;
constexpr double MIN_TIME_TO_EXPIRY = 1e-10;
}

// Calculate all Greeks for a call option
OptionGreeks BlackScholesGreeks::calculateCallGreeks(
    double spot_price,
    double strike_price,
    double risk_free_rate,
    double volatility,
    double time_to_expiry
) {
    OptionGreeks greeks;
    if (spot_price <= 0 || strike_price <= 0 || time_to_expiry < MIN_TIME_TO_EXPIRY || volatility < MIN_VOLATILITY) {
        return greeks;
    }

    const double sigma_sqrt_T = volatility * std::sqrt(time_to_expiry);
    double d1 = (std::log(spot_price / strike_price) +
                 (risk_free_rate + 0.5 * volatility * volatility) * time_to_expiry)
                / sigma_sqrt_T;
    double d2 = d1 - sigma_sqrt_T;

    greeks.delta = standardNormalCDF(d1);
    greeks.gamma = standardNormalPDF(d1) / (spot_price * sigma_sqrt_T);
    double theta = -spot_price * standardNormalPDF(d1) * volatility / (2 * std::sqrt(time_to_expiry))
                   - risk_free_rate * strike_price * std::exp(-risk_free_rate * time_to_expiry)
                   * standardNormalCDF(d2);
    greeks.theta = theta / 365.0;
    greeks.vega = spot_price * std::sqrt(time_to_expiry) * standardNormalPDF(d1) / 100.0;
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
    if (spot_price <= 0 || strike_price <= 0 || time_to_expiry < MIN_TIME_TO_EXPIRY || volatility < MIN_VOLATILITY) {
        return greeks;
    }

    const double sigma_sqrt_T = volatility * std::sqrt(time_to_expiry);
    double d1 = (std::log(spot_price / strike_price) +
                 (risk_free_rate + 0.5 * volatility * volatility) * time_to_expiry)
                / sigma_sqrt_T;
    double d2 = d1 - sigma_sqrt_T;

    greeks.delta = standardNormalCDF(d1) - 1;
    greeks.gamma = standardNormalPDF(d1) / (spot_price * sigma_sqrt_T);
    double theta = -spot_price * standardNormalPDF(d1) * volatility / (2 * std::sqrt(time_to_expiry))
                   + risk_free_rate * strike_price * std::exp(-risk_free_rate * time_to_expiry)
                   * standardNormalCDF(-d2);
    greeks.theta = theta / 365.0;
    greeks.vega = spot_price * std::sqrt(time_to_expiry) * standardNormalPDF(d1) / 100.0;
    greeks.rho = -strike_price * time_to_expiry * std::exp(-risk_free_rate * time_to_expiry)
                 * standardNormalCDF(-d2) / 100.0;

    return greeks;
}