#ifndef BLACK_SCHOLES_GREEKS_H
#define BLACK_SCHOLES_GREEKS_H

#include <cmath>

// Forward declaration of existing BlackScholes functions
class BlackScholes;

// Structure to hold all Greek values for an option
struct OptionGreeks {
    double delta;  // Change in option price for a $1 change in underlying price
    double gamma;  // Rate of change of delta with respect to underlying price
    double theta;  // Rate of change of option value with respect to time (time decay)
    double vega;   // Rate of change of option value with respect to volatility
    double rho;    // Rate of change of option value with respect to interest rate
};

class BlackScholesGreeks {
public:
    // Calculate all Greeks for a call option
    static OptionGreeks calculateCallGreeks(
        double spot_price,
        double strike_price,
        double risk_free_rate,
        double volatility,
        double time_to_expiry
    );

    // Calculate all Greeks for a put option
    static OptionGreeks calculatePutGreeks(
        double spot_price,
        double strike_price,
        double risk_free_rate,
        double volatility,
        double time_to_expiry
    );

private:
    // Standard Normal Probability Density Function (reused from your existing code)
    static double standardNormalPDF(double x) {
        return (1.0 / std::sqrt(2.0 * M_PI)) * std::exp(-0.5 * x * x);
    }

    // Standard Normal Cumulative Distribution Function (reused from your existing code)
    static double standardNormalCDF(double x) {
        return 0.5 * (1.0 + std::erf(x / std::sqrt(2.0)));
    }
};

#endif // BLACK_SCHOLES_GREEKS_H