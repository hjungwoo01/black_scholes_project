#ifndef PAPER_TRADING_H
#define PAPER_TRADING_H

#include "option.h"
#include "market_data.h"
#include <vector>
#include <unordered_map>
#include <string>
#include <ctime>

struct OptionPosition {
    Option option;
    int quantity;
    double entry_price;
    std::time_t entry_time;
};

class PaperTradingSystem {
private:
    double cash_balance;
    double initial_balance;
    std::vector<OptionPosition> open_positions;
    MarketDataProvider market_data;

public:
    // Constructor with initial balance and API key
    PaperTradingSystem(double initial_balance, const std::string& alpha_vantage_api_key);

    // Trading operations
    bool buyOption(const Option& option, int quantity);
    bool sellOption(const Option& option, int quantity);

    // Update option fair-value prices from market data (underlying price -> Black-Scholes)
    void updateOptionPricesFromMarket(double risk_free_rate, double volatility);

    // Expose market data for pushing prices (e.g. setCurrentPrice after API fetch)
    MarketDataProvider& getMarketData();

    // Portfolio management
    double calculatePortfolioValue() const;
    void closeExpiredPositions();

    // Reporting
    void printPortfolio() const;
    double getCashBalance() const;
};

#endif // PAPER_TRADING_H