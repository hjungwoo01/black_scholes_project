#ifndef MONTE_CARLO_H
#define MONTE_CARLO_H

#include <cmath>
#include <vector>
#include <functional>
#include <cstddef>

/** Progress callback: (done, total) for UI updates. */
using MonteCarloProgressCallback = std::function<void(int done, int total)>;

class MonteCarloOptionPricer {
public:
    /** Call: single-threaded, no progress. */
    static double priceCallOption(
        double spot_price,
        double strike_price,
        double risk_free_rate,
        double volatility,
        double time_to_expiry,
        int num_simulations = 10000
    );

    /** Call: optional progress callback; uses std::async for parallelism. */
    static double priceCallOption(
        double spot_price,
        double strike_price,
        double risk_free_rate,
        double volatility,
        double time_to_expiry,
        int num_simulations,
        MonteCarloProgressCallback progress_cb
    );

    /** Put: single-threaded, no progress. */
    static double pricePutOption(
        double spot_price,
        double strike_price,
        double risk_free_rate,
        double volatility,
        double time_to_expiry,
        int num_simulations = 10000
    );

    /** Put: optional progress callback; uses std::async for parallelism. */
    static double pricePutOption(
        double spot_price,
        double strike_price,
        double risk_free_rate,
        double volatility,
        double time_to_expiry,
        int num_simulations,
        MonteCarloProgressCallback progress_cb
    );

    static std::vector<std::vector<double>> simulatePricePaths(
        double spot_price,
        double risk_free_rate,
        double volatility,
        double time_to_expiry,
        int num_paths = 100,
        int steps_per_path = 252
    );
};

#endif
