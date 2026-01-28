#include "../include/black_scholes.h"
#include <stdexcept>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace {
constexpr double MIN_VOLATILITY = 1e-10;
constexpr double MIN_TIME_TO_EXPIRY = 1e-10;
}

// Standard Normal Cumulative Distribution Function
double BlackScholes::standardNormalCDF(double x) {
    return 0.5 * (1.0 + std::erf(x / std::sqrt(2.0)));
}

// Standard Normal Probability Density Function
double BlackScholes::standardNormalPDF(double x) {
    return (1.0 / std::sqrt(2.0 * M_PI)) * std::exp(-0.5 * x * x);
}

// Intrinsic value at expiry (T=0): Call = max(S-K,0), Put = max(K-S,0)
static double callIntrinsic(double spot_price, double strike_price) {
    return std::max(spot_price - strike_price, 0.0);
}
static double putIntrinsic(double spot_price, double strike_price) {
    return std::max(strike_price - spot_price, 0.0);
}

// Calculate Call Option Price
// d1 = [ln(S/K) + (r + σ²/2)T] / (σ√T),  d2 = d1 - σ√T
double BlackScholes::calculateCallPrice(
    double spot_price,
    double strike_price,
    double risk_free_rate,
    double volatility,
    double time_to_expiry
) {
    if (spot_price <= 0 || strike_price <= 0) {
        throw std::invalid_argument("Invalid input: spot and strike must be positive");
    }
    if (time_to_expiry < 0) {
        throw std::invalid_argument("Invalid input: time_to_expiry must be non-negative");
    }

    // T = 0 or effectively zero: return intrinsic value
    if (time_to_expiry < MIN_TIME_TO_EXPIRY) {
        return callIntrinsic(spot_price, strike_price);
    }
    // σ = 0 or effectively zero: no volatility, return discounted intrinsic
    if (volatility < MIN_VOLATILITY) {
        double forward = spot_price * std::exp(risk_free_rate * time_to_expiry);
        return std::exp(-risk_free_rate * time_to_expiry) * std::max(forward - strike_price, 0.0);
    }

    const double sigma_sq = volatility * volatility;
    const double sigma_sqrt_T = volatility * std::sqrt(time_to_expiry);
    double d1 = (std::log(spot_price / strike_price) +
                 (risk_free_rate + 0.5 * sigma_sq) * time_to_expiry)
                / sigma_sqrt_T;
    double d2 = d1 - sigma_sqrt_T;

    return spot_price * standardNormalCDF(d1) -
           strike_price * std::exp(-risk_free_rate * time_to_expiry) * standardNormalCDF(d2);
}

// Calculate Put Option Price
double BlackScholes::calculatePutPrice(
    double spot_price,
    double strike_price,
    double risk_free_rate,
    double volatility,
    double time_to_expiry
) {
    if (spot_price <= 0 || strike_price <= 0) {
        throw std::invalid_argument("Invalid input: spot and strike must be positive");
    }
    if (time_to_expiry < 0) {
        throw std::invalid_argument("Invalid input: time_to_expiry must be non-negative");
    }

    if (time_to_expiry < MIN_TIME_TO_EXPIRY) {
        return putIntrinsic(spot_price, strike_price);
    }
    if (volatility < MIN_VOLATILITY) {
        double forward = spot_price * std::exp(risk_free_rate * time_to_expiry);
        return std::exp(-risk_free_rate * time_to_expiry) * std::max(strike_price - forward, 0.0);
    }

    const double sigma_sq = volatility * volatility;
    const double sigma_sqrt_T = volatility * std::sqrt(time_to_expiry);
    double d1 = (std::log(spot_price / strike_price) +
                 (risk_free_rate + 0.5 * sigma_sq) * time_to_expiry)
                / sigma_sqrt_T;
    double d2 = d1 - sigma_sqrt_T;

    return strike_price * std::exp(-risk_free_rate * time_to_expiry) * standardNormalCDF(-d2) -
           spot_price * standardNormalCDF(-d1);
}

namespace {
// Bisection fallback when Newton-Raphson fails (e.g. vega too small)
double impliedVolatilityBisection(double market_price, double spot_price, double strike_price,
                                  double risk_free_rate, double time_to_expiry, OptionType option_type) {
    const double vol_lo = 0.0001;
    const double vol_hi = 5.0;
    const int max_it = 80;
    const double tol = 1e-5;
    double a = vol_lo, b = vol_hi;
    auto model = [&](double sig) {
        if (option_type == CALL)
            return BlackScholes::calculateCallPrice(spot_price, strike_price, risk_free_rate, sig, time_to_expiry);
        return BlackScholes::calculatePutPrice(spot_price, strike_price, risk_free_rate, sig, time_to_expiry);
    };
    double fa = model(a) - market_price;
    double fb = model(b) - market_price;
    if (fa * fb > 0) return 0.3;
    for (int i = 0; i < max_it; ++i) {
        double c = 0.5 * (a + b);
        if ((b - a) * 0.5 < tol) return c;
        double fc = model(c) - market_price;
        if (std::abs(fc) < tol) return c;
        if (fa * fc < 0) { b = c; fb = fc; }
        else { a = c; fa = fc; }
    }
    return 0.5 * (a + b);
}
}

// Implied Volatility: Newton-Raphson with Bisection fallback
double BlackScholes::calculateImpliedVolatility(
    double market_price,
    double spot_price,
    double strike_price,
    double risk_free_rate,
    double time_to_expiry,
    OptionType option_type
) {
    if (spot_price <= 0 || strike_price <= 0 || time_to_expiry < MIN_TIME_TO_EXPIRY || market_price < 0) {
        throw std::invalid_argument("Invalid input for implied volatility");
    }

    double volatility = 0.3;
    const int max_iterations = 100;
    const double tolerance = 1e-5;

    for (int i = 0; i < max_iterations; ++i) {
        if (volatility < MIN_VOLATILITY) volatility = MIN_VOLATILITY;

        double model_price;
        double d1 = (std::log(spot_price / strike_price) +
                     (risk_free_rate + 0.5 * volatility * volatility) * time_to_expiry)
                    / (volatility * std::sqrt(time_to_expiry));
        double vega = spot_price * standardNormalPDF(d1) * std::sqrt(time_to_expiry);

        if (option_type == CALL) {
            model_price = calculateCallPrice(spot_price, strike_price, risk_free_rate, volatility, time_to_expiry);
        } else {
            model_price = calculatePutPrice(spot_price, strike_price, risk_free_rate, volatility, time_to_expiry);
        }

        double price_diff = model_price - market_price;
        if (std::abs(price_diff) < tolerance) {
            return volatility;
        }
        if (vega < 1e-15) {
            return impliedVolatilityBisection(market_price, spot_price, strike_price, risk_free_rate, time_to_expiry, option_type);
        }
        volatility -= price_diff / vega;
        volatility = std::max(0.0001, std::min(volatility, 5.0));
    }
    return impliedVolatilityBisection(market_price, spot_price, strike_price, risk_free_rate, time_to_expiry, option_type);
}