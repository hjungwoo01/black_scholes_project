#ifndef BLACK_SCHOLES_H
#define BLACK_SCHOLES_H

#include "option.h"
#include <cmath>

class BlackScholes {
public:
    static double calculateCallPrice(
        double spot_price, 
        double strike_price, 
        double risk_free_rate, 
        double volatility, 
        double time_to_expiry
    );

    static double calculatePutPrice(
        double spot_price, 
        double strike_price, 
        double risk_free_rate, 
        double volatility, 
        double time_to_expiry
    );

    static double calculateImpliedVolatility(
        double market_price,
        double spot_price,
        double strike_price,
        double risk_free_rate,
        double time_to_expiry,
        OptionType option_type
    );

private:
    static double standardNormalCDF(double x);
    static double standardNormalPDF(double x);
};

#endif // BLACK_SCHOLES_H