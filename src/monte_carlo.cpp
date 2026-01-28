#include "monte_carlo.h"
#include <random>
#include <future>
#include <algorithm>
#include <thread>

namespace {

constexpr int MIN_CHUNK = 500;
constexpr double TRADING_DAYS_PER_YEAR = 252.0;

double runCallChunk(double spot, double strike, double r, double vol, double T,
                   int sim_begin, int sim_end, unsigned seed) noexcept {
    const double dt = T / TRADING_DAYS_PER_YEAR;
    const double sqrt_dt = std::sqrt(dt);
    const int steps = static_cast<int>(TRADING_DAYS_PER_YEAR * T);
    std::mt19937 gen(seed);
    std::normal_distribution<> dist(0.0, 1.0);
    double sum = 0.0;
    for (int i = sim_begin; i < sim_end; ++i) {
        (void)i;
        double S = spot;
        for (int t = 0; t < steps; ++t) {
            double z = dist(gen);
            S *= std::exp((r - 0.5 * vol * vol) * dt + vol * sqrt_dt * z);
        }
        sum += std::max(S - strike, 0.0);
    }
    return sum;
}

double runPutChunk(double spot, double strike, double r, double vol, double T,
                  int sim_begin, int sim_end, unsigned seed) noexcept {
    const double dt = T / TRADING_DAYS_PER_YEAR;
    const double sqrt_dt = std::sqrt(dt);
    const int steps = static_cast<int>(TRADING_DAYS_PER_YEAR * T);
    std::mt19937 gen(seed);
    std::normal_distribution<> dist(0.0, 1.0);
    double sum = 0.0;
    for (int i = sim_begin; i < sim_end; ++i) {
        (void)i;
        double S = spot;
        for (int t = 0; t < steps; ++t) {
            double z = dist(gen);
            S *= std::exp((r - 0.5 * vol * vol) * dt + vol * sqrt_dt * z);
        }
        sum += std::max(strike - S, 0.0);
    }
    return sum;
}

} // namespace

double MonteCarloOptionPricer::priceCallOption(
    double spot_price, double strike_price, double risk_free_rate,
    double volatility, double time_to_expiry, int num_simulations
) {
    return priceCallOption(spot_price, strike_price, risk_free_rate, volatility,
                           time_to_expiry, num_simulations, MonteCarloProgressCallback{});
}

double MonteCarloOptionPricer::priceCallOption(
    double spot_price, double strike_price, double risk_free_rate,
    double volatility, double time_to_expiry, int num_simulations,
    MonteCarloProgressCallback progress_cb
) {
    int n = std::max(1, num_simulations);
    unsigned n_workers = static_cast<unsigned>(std::max(1, static_cast<int>(std::thread::hardware_concurrency())));
    int chunk_size = std::max(MIN_CHUNK, static_cast<int>(n / n_workers));
    std::vector<std::future<double>> futures;
    int done = 0;
    std::random_device rd;
    for (int start = 0; start < n; ) {
        int end = std::min(start + chunk_size, n);
        unsigned seed = rd();
        futures.push_back(std::async(std::launch::async, runCallChunk,
                                     spot_price, strike_price, risk_free_rate, volatility, time_to_expiry,
                                     start, end, seed));
        start = end;
    }
    double sum_payoffs = 0.0;
    for (size_t i = 0; i < futures.size(); ++i) {
        sum_payoffs += futures[i].get();
        done = std::min(static_cast<int>((i + 1) * chunk_size), n);
        if (progress_cb) progress_cb(done, n);
    }
    return std::exp(-risk_free_rate * time_to_expiry) * (sum_payoffs / n);
}

double MonteCarloOptionPricer::pricePutOption(
    double spot_price, double strike_price, double risk_free_rate,
    double volatility, double time_to_expiry, int num_simulations
) {
    return pricePutOption(spot_price, strike_price, risk_free_rate, volatility,
                          time_to_expiry, num_simulations, MonteCarloProgressCallback{});
}

double MonteCarloOptionPricer::pricePutOption(
    double spot_price, double strike_price, double risk_free_rate,
    double volatility, double time_to_expiry, int num_simulations,
    MonteCarloProgressCallback progress_cb
) {
    int n = std::max(1, num_simulations);
    unsigned n_workers = static_cast<unsigned>(std::max(1, static_cast<int>(std::thread::hardware_concurrency())));
    int chunk_size = std::max(MIN_CHUNK, static_cast<int>(n / n_workers));
    std::vector<std::future<double>> futures;
    int done = 0;
    std::random_device rd;
    for (int start = 0; start < n; ) {
        int end = std::min(start + chunk_size, n);
        unsigned seed = rd();
        futures.push_back(std::async(std::launch::async, runPutChunk,
                                     spot_price, strike_price, risk_free_rate, volatility, time_to_expiry,
                                     start, end, seed));
        start = end;
    }
    double sum_payoffs = 0.0;
    for (size_t i = 0; i < futures.size(); ++i) {
        sum_payoffs += futures[i].get();
        done = std::min(static_cast<int>((i + 1) * chunk_size), n);
        if (progress_cb) progress_cb(done, n);
    }
    return std::exp(-risk_free_rate * time_to_expiry) * (sum_payoffs / n);
}

std::vector<std::vector<double>> MonteCarloOptionPricer::simulatePricePaths(
    double spot_price, double risk_free_rate, double volatility,
    double time_to_expiry, int num_paths, int steps_per_path
) {
    const double dt = time_to_expiry / steps_per_path;
    const double sqrt_dt = std::sqrt(dt);
    std::random_device rd;
    std::mt19937 gen(rd());
    std::normal_distribution<> dist(0.0, 1.0);
    std::vector<std::vector<double>> paths(static_cast<size_t>(num_paths),
                                           std::vector<double>(static_cast<size_t>(steps_per_path + 1), 0.0));
    for (int i = 0; i < num_paths; ++i)
        paths[static_cast<size_t>(i)][0] = spot_price;
    for (int i = 0; i < num_paths; ++i) {
        for (int t = 1; t <= steps_per_path; ++t) {
            double z = dist(gen);
            paths[static_cast<size_t>(i)][static_cast<size_t>(t)] =
                paths[static_cast<size_t>(i)][static_cast<size_t>(t - 1)] * std::exp(
                    (risk_free_rate - 0.5 * volatility * volatility) * dt + volatility * sqrt_dt * z);
        }
    }
    return paths;
}
