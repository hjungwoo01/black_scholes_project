#ifndef OPTION_STRATEGIES_H
#define OPTION_STRATEGIES_H

#include <vector>
#include <string>
#include <memory>
#include <cmath>
#include <algorithm>
#include "../include/option.h"
#include "../include/black_scholes.h"
#include "../include/black_scholes_greeks.h"

// Forward declaration
class OptionStrategy;

// Factory for creating different option strategies
class OptionStrategyFactory {
public:
    enum StrategyType {
        COVERED_CALL,
        PROTECTIVE_PUT,
        BULL_CALL_SPREAD,
        BEAR_PUT_SPREAD,
        STRADDLE,
        STRANGLE,
        IRON_CONDOR,
        BUTTERFLY
    };
    
    static std::unique_ptr<OptionStrategy> createStrategy(
        StrategyType type,
        const std::string& symbol,
        double current_price,
        double volatility,
        double risk_free_rate,
        std::time_t expiry
    );
};

// Base class for option strategies
class OptionStrategy {
protected:
    std::string symbol;
    std::vector<Option> options;
    int stock_position = 0; // Number of shares (positive for long, negative for short)
    double entry_price = 0.0; // Cost basis for the strategy
    
public:
    OptionStrategy(const std::string& symbol) : symbol(symbol) {}
    virtual ~OptionStrategy() = default;
    
    // Calculate the strategy's value at a given underlying price
    virtual double calculateValue(double underlying_price, double time_to_expiry) const = 0;
    
    // Calculate the strategy's delta at a given underlying price
    virtual double calculateDelta(double underlying_price, double time_to_expiry, 
                                 double risk_free_rate, double volatility) const = 0;
    
    // Calculate maximum profit
    virtual double calculateMaxProfit() const = 0;
    
    // Calculate maximum loss
    virtual double calculateMaxLoss() const = 0;
    
    // Calculate breakeven points
    virtual std::vector<double> calculateBreakevens() const = 0;
    
    // Get all options in the strategy
    const std::vector<Option>& getOptions() const { return options; }
    
    // Get stock position
    int getStockPosition() const { return stock_position; }
    
    // Get symbol
    std::string getSymbol() const { return symbol; }

    // Get entry price
    double getEntryPrice() const { return entry_price; }
    
    // Add option to the strategy
    void addOption(const Option& option) {
        options.push_back(option);
    }
    
    // Set stock position
    void setStockPosition(int position) {
        stock_position = position;
    }

    // Set entry price
    void setEntryPrice(double price) {
        entry_price = price;
    }
};

// CoveredCall Strategy Implementation
class CoveredCallStrategy : public OptionStrategy {
private:
    double entry_stock_price;
    double call_premium;
    
public:
    CoveredCallStrategy(
        const std::string& symbol,
        double current_price,
        double strike_price,
        double volatility,
        double risk_free_rate,
        std::time_t expiry
    ) : OptionStrategy(symbol), entry_stock_price(current_price) {
        // Long 100 shares of stock
        stock_position = 100;
        
        // Short 1 call option
        Option call_option(symbol, CALL, strike_price, expiry);
        
        // Calculate the call premium
        double time_to_expiry = difftime(expiry, std::time(nullptr)) / (60 * 60 * 24 * 365); // Convert to years
        call_premium = BlackScholes::calculateCallPrice(
            current_price, strike_price, risk_free_rate, volatility, time_to_expiry
        );
        
        call_option.setCurrentPrice(call_premium);
        addOption(call_option);
        
        // Set the entry price (cost basis minus premium received)
        entry_price = entry_stock_price * 100 - call_premium * 100;
    }
    
    double calculateValue(double underlying_price, double time_to_expiry) const override {
        double stock_value = stock_position * underlying_price;
        double option_value = 0.0;
        
        // Calculate the current value of the short call
        // Since it's short, we subtract its value
        for (const auto& option : options) {
            if (option.getType() == CALL) {
                double call_price = BlackScholes::calculateCallPrice(
                    underlying_price, option.getStrikePrice(), 0.02, 0.3, time_to_expiry
                );
                option_value -= call_price * 100; // 100 shares per contract
            }
        }
        
        return stock_value + option_value;
    }
    
    double calculateDelta(double underlying_price, double time_to_expiry, 
                       double risk_free_rate, double volatility) const override {
        // Delta of stock position = 1 per share
        double delta = stock_position;
        
        for (const auto& option : options) {
            if (option.getType() == CALL) {
                // Get the delta of the call option
                OptionGreeks greeks = BlackScholesGreeks::calculateCallGreeks(
                    underlying_price, option.getStrikePrice(), risk_free_rate, volatility, time_to_expiry
                );
                // Subtract the delta because we're short the call
                delta -= greeks.delta * 100; // 100 shares per contract
            }
        }
        
        return delta;
    }
    
    double calculateMaxProfit() const override {
        // Max profit for covered call is limited to:
        // (strike price - entry stock price) * shares + call premium * shares
        double max_profit_per_share = 0.0;
        
        for (const auto& option : options) {
            if (option.getType() == CALL) {
                max_profit_per_share = option.getStrikePrice() - entry_stock_price + option.getCurrentPrice();
                break;
            }
        }
        
        return max_profit_per_share * std::abs(stock_position);
    }
    
    double calculateMaxLoss() const override {
        // Max loss is if stock goes to zero, minus the premium received
        // (entry stock price * shares) - (call premium * shares)
        return entry_price;
    }
    
    std::vector<double> calculateBreakevens() const override {
        // Breakeven point is: 
        // entry stock price - call premium
        double breakeven = entry_stock_price - call_premium;
        return {breakeven};
    }
};

// Protective Put Strategy Implementation
class ProtectivePutStrategy : public OptionStrategy {
private:
    double entry_stock_price;
    double put_premium;
    
public:
    ProtectivePutStrategy(
        const std::string& symbol,
        double current_price,
        double strike_price,
        double volatility,
        double risk_free_rate,
        std::time_t expiry
    ) : OptionStrategy(symbol), entry_stock_price(current_price) {
        // Long 100 shares of stock
        stock_position = 100;
        
        // Long 1 put option
        Option put_option(symbol, PUT, strike_price, expiry);
        
        // Calculate the put premium
        double time_to_expiry = difftime(expiry, std::time(nullptr)) / (60 * 60 * 24 * 365); // Convert to years
        put_premium = BlackScholes::calculatePutPrice(
            current_price, strike_price, risk_free_rate, volatility, time_to_expiry
        );
        
        put_option.setCurrentPrice(put_premium);
        addOption(put_option);
        
        // Set the entry price (cost basis plus premium paid)
        entry_price = entry_stock_price * 100 + put_premium * 100;
    }
    
    double calculateValue(double underlying_price, double time_to_expiry) const override {
        double stock_value = stock_position * underlying_price;
        double option_value = 0.0;
        
        // Calculate the current value of the long put
        for (const auto& option : options) {
            if (option.getType() == PUT) {
                double put_price = BlackScholes::calculatePutPrice(
                    underlying_price, option.getStrikePrice(), 0.02, 0.3, time_to_expiry
                );
                option_value += put_price * 100; // 100 shares per contract
            }
        }
        
        return stock_value + option_value;
    }
    
    double calculateDelta(double underlying_price, double time_to_expiry, 
                       double risk_free_rate, double volatility) const override {
        // Delta of stock position = 1 per share
        double delta = stock_position;
        
        for (const auto& option : options) {
            if (option.getType() == PUT) {
                // Get the delta of the put option
                OptionGreeks greeks = BlackScholesGreeks::calculatePutGreeks(
                    underlying_price, option.getStrikePrice(), risk_free_rate, volatility, time_to_expiry
                );
                // Add the delta because we're long the put
                delta += greeks.delta * 100; // 100 shares per contract
            }
        }
        
        return delta;
    }
    
    double calculateMaxProfit() const override {
        // Theoretically unlimited (if stock price increases infinitely)
        // In practice, we can estimate a reasonable maximum
        // Max profit = (very high price - entry price) * shares - put premium
        // For simplicity, assume max profit can be 3x the current price
        return (entry_stock_price * 3 - entry_stock_price) * stock_position - put_premium * 100;
    }
    
    double calculateMaxLoss() const override {
        // Max loss is limited to:
        // (entry stock price - put strike price) * shares + put premium * shares
        double max_loss_per_share = 0.0;
        
        for (const auto& option : options) {
            if (option.getType() == PUT) {
                max_loss_per_share = entry_stock_price - option.getStrikePrice() + option.getCurrentPrice();
                break;
            }
        }
        
        return max_loss_per_share * std::abs(stock_position);
    }
    
    std::vector<double> calculateBreakevens() const override {
        // Breakeven point is: 
        // entry stock price + put premium
        double breakeven = entry_stock_price + put_premium;
        return {breakeven};
    }
};

// Bull Call Spread Strategy Implementation
class BullCallSpreadStrategy : public OptionStrategy {
private:
    double long_call_strike;
    double short_call_strike;
    double long_call_premium;
    double short_call_premium;
    
public:
    BullCallSpreadStrategy(
        const std::string& symbol,
        double current_price,
        double long_strike,
        double short_strike,
        double volatility,
        double risk_free_rate,
        std::time_t expiry
    ) : OptionStrategy(symbol), long_call_strike(long_strike), short_call_strike(short_strike) {
        if (long_strike >= short_strike) {
            throw std::invalid_argument("Long call strike must be lower than short call strike for a bull call spread");
        }
        
        // Calculate time to expiry
        double time_to_expiry = difftime(expiry, std::time(nullptr)) / (60 * 60 * 24 * 365); // Convert to years
        
        // Long 1 call option (lower strike)
        Option long_call(symbol, CALL, long_strike, expiry);
        long_call_premium = BlackScholes::calculateCallPrice(
            current_price, long_strike, risk_free_rate, volatility, time_to_expiry
        );
        long_call.setCurrentPrice(long_call_premium);
        addOption(long_call);
        
        // Short 1 call option (higher strike)
        Option short_call(symbol, CALL, short_strike, expiry);
        short_call_premium = BlackScholes::calculateCallPrice(
            current_price, short_strike, risk_free_rate, volatility, time_to_expiry
        );
        short_call.setCurrentPrice(short_call_premium);
        addOption(short_call);
        
        // Set the entry price (net debit)
        entry_price = (long_call_premium - short_call_premium) * 100;
    }
    
    double calculateValue(double underlying_price, double time_to_expiry) const override {
        double option_value = 0.0;
        
        // Long call value
        double long_call_value = BlackScholes::calculateCallPrice(
            underlying_price, long_call_strike, 0.02, 0.3, time_to_expiry
        );
        
        // Short call value
        double short_call_value = BlackScholes::calculateCallPrice(
            underlying_price, short_call_strike, 0.02, 0.3, time_to_expiry
        );
        
        // Total value = long call value - short call value
        option_value = (long_call_value - short_call_value) * 100;
        
        return option_value;
    }
    
    double calculateDelta(double underlying_price, double time_to_expiry, 
                       double risk_free_rate, double volatility) const override {
        // Long call delta
        OptionGreeks long_call_greeks = BlackScholesGreeks::calculateCallGreeks(
            underlying_price, long_call_strike, risk_free_rate, volatility, time_to_expiry
        );
        
        // Short call delta
        OptionGreeks short_call_greeks = BlackScholesGreeks::calculateCallGreeks(
            underlying_price, short_call_strike, risk_free_rate, volatility, time_to_expiry
        );
        
        // Net delta = long call delta - short call delta
        return (long_call_greeks.delta - short_call_greeks.delta) * 100;
    }
    
    double calculateMaxProfit() const override {
        // Max profit = (short call strike - long call strike) * 100 - net premium paid
        return (short_call_strike - long_call_strike) * 100 - entry_price;
    }
    
    double calculateMaxLoss() const override {
        // Max loss is limited to the net premium paid (entry price)
        return entry_price;
    }
    
    std::vector<double> calculateBreakevens() const override {
        // Breakeven point = long call strike + net premium per share
        double net_premium_per_share = entry_price / 100;
        double breakeven = long_call_strike + net_premium_per_share;
        return {breakeven};
    }
};

// Bear Put Spread Strategy Implementation
class BearPutSpreadStrategy : public OptionStrategy {
private:
    double long_put_strike;
    double short_put_strike;
    double long_put_premium;
    double short_put_premium;
    
public:
    BearPutSpreadStrategy(
        const std::string& symbol,
        double current_price,
        double long_strike,
        double short_strike,
        double volatility,
        double risk_free_rate,
        std::time_t expiry
    ) : OptionStrategy(symbol), long_put_strike(long_strike), short_put_strike(short_strike) {
        if (long_strike <= short_strike) {
            throw std::invalid_argument("Long put strike must be higher than short put strike for a bear put spread");
        }
        
        // Calculate time to expiry
        double time_to_expiry = difftime(expiry, std::time(nullptr)) / (60 * 60 * 24 * 365); // Convert to years
        
        // Long 1 put option (higher strike)
        Option long_put(symbol, PUT, long_strike, expiry);
        long_put_premium = BlackScholes::calculatePutPrice(
            current_price, long_strike, risk_free_rate, volatility, time_to_expiry
        );
        long_put.setCurrentPrice(long_put_premium);
        addOption(long_put);
        
        // Short 1 put option (lower strike)
        Option short_put(symbol, PUT, short_strike, expiry);
        short_put_premium = BlackScholes::calculatePutPrice(
            current_price, short_strike, risk_free_rate, volatility, time_to_expiry
        );
        short_put.setCurrentPrice(short_put_premium);
        addOption(short_put);
        
        // Set the entry price (net debit)
        entry_price = (long_put_premium - short_put_premium) * 100;
    }
    
    double calculateValue(double underlying_price, double time_to_expiry) const override {
        double option_value = 0.0;
        
        // Long put value
        double long_put_value = BlackScholes::calculatePutPrice(
            underlying_price, long_put_strike, 0.02, 0.3, time_to_expiry
        );
        
        // Short put value
        double short_put_value = BlackScholes::calculatePutPrice(
            underlying_price, short_put_strike, 0.02, 0.3, time_to_expiry
        );
        
        // Total value = long put value - short put value
        option_value = (long_put_value - short_put_value) * 100;
        
        return option_value;
    }
    
    double calculateDelta(double underlying_price, double time_to_expiry, 
                       double risk_free_rate, double volatility) const override {
        // Long put delta
        OptionGreeks long_put_greeks = BlackScholesGreeks::calculatePutGreeks(
            underlying_price, long_put_strike, risk_free_rate, volatility, time_to_expiry
        );
        
        // Short put delta
        OptionGreeks short_put_greeks = BlackScholesGreeks::calculatePutGreeks(
            underlying_price, short_put_strike, risk_free_rate, volatility, time_to_expiry
        );
        
        // Net delta = long put delta - short put delta
        return (long_put_greeks.delta - short_put_greeks.delta) * 100;
    }
    
    double calculateMaxProfit() const override {
        // Max profit = (long put strike - short put strike) * 100 - net premium paid
        return (long_put_strike - short_put_strike) * 100 - entry_price;
    }
    
    double calculateMaxLoss() const override {
        // Max loss is limited to the net premium paid (entry price)
        return entry_price;
    }
    
    std::vector<double> calculateBreakevens() const override {
        // Breakeven point = long put strike - net premium per share
        double net_premium_per_share = entry_price / 100;
        double breakeven = long_put_strike - net_premium_per_share;
        return {breakeven};
    }
};

// Straddle Strategy Implementation
class StraddleStrategy : public OptionStrategy {
private:
    double strike_price;
    double call_premium;
    double put_premium;
    
public:
    StraddleStrategy(
        const std::string& symbol,
        double current_price,
        double strike,
        double volatility,
        double risk_free_rate,
        std::time_t expiry
    ) : OptionStrategy(symbol), strike_price(strike) {
        // Calculate time to expiry
        double time_to_expiry = difftime(expiry, std::time(nullptr)) / (60 * 60 * 24 * 365); // Convert to years
        
        // Long 1 call option
        Option call(symbol, CALL, strike, expiry);
        call_premium = BlackScholes::calculateCallPrice(
            current_price, strike, risk_free_rate, volatility, time_to_expiry
        );
        call.setCurrentPrice(call_premium);
        addOption(call);
        
        // Long 1 put option
        Option put(symbol, PUT, strike, expiry);
        put_premium = BlackScholes::calculatePutPrice(
            current_price, strike, risk_free_rate, volatility, time_to_expiry
        );
        put.setCurrentPrice(put_premium);
        addOption(put);
        
        // Set the entry price (total premium paid)
        entry_price = (call_premium + put_premium) * 100;
    }
    
    double calculateValue(double underlying_price, double time_to_expiry) const override {
        double option_value = 0.0;
        
        // Call value
        double call_value = BlackScholes::calculateCallPrice(
            underlying_price, strike_price, 0.02, 0.3, time_to_expiry
        );
        
        // Put value
        double put_value = BlackScholes::calculatePutPrice(
            underlying_price, strike_price, 0.02, 0.3, time_to_expiry
        );
        
        // Total value = call value + put value
        option_value = (call_value + put_value) * 100;
        
        return option_value;
    }
    
    double calculateDelta(double underlying_price, double time_to_expiry, 
                       double risk_free_rate, double volatility) const override {
        // Call delta
        OptionGreeks call_greeks = BlackScholesGreeks::calculateCallGreeks(
            underlying_price, strike_price, risk_free_rate, volatility, time_to_expiry
        );
        
        // Put delta
        OptionGreeks put_greeks = BlackScholesGreeks::calculatePutGreeks(
            underlying_price, strike_price, risk_free_rate, volatility, time_to_expiry
        );
        
        // Net delta = call delta + put delta
        return (call_greeks.delta + put_greeks.delta) * 100;
    }
    
    double calculateMaxProfit() const override {
        // Theoretically unlimited (if stock price moves far enough in either direction)
        // For practical purposes, we can estimate a large move
        // Max profit = abs(very high/low price - strike) - total premium
        double potential_upside = std::max(strike_price * 2 - strike_price, strike_price * 0.5);
        return potential_upside * 100 - entry_price;
    }
    
    double calculateMaxLoss() const override {
        // Max loss occurs at expiration if the stock price equals the strike price
        // Max loss = total premium paid (entry price)
        return entry_price;
    }
    
    std::vector<double> calculateBreakevens() const override {
        // Two breakeven points:
        // Lower breakeven = strike price - total premium per share
        // Upper breakeven = strike price + total premium per share
        double total_premium_per_share = entry_price / 100;
        double lower_breakeven = strike_price - total_premium_per_share;
        double upper_breakeven = strike_price + total_premium_per_share;
        return {lower_breakeven, upper_breakeven};
    }
};

// Implementation of the factory method
std::unique_ptr<OptionStrategy> OptionStrategyFactory::createStrategy(
    StrategyType type,
    const std::string& symbol,
    double current_price,
    double volatility,
    double risk_free_rate,
    std::time_t expiry
) {
    switch (type) {
        case COVERED_CALL:
            return std::make_unique<CoveredCallStrategy>(
                symbol, current_price, current_price * 1.05, volatility, risk_free_rate, expiry
            );
            
        case PROTECTIVE_PUT:
            return std::make_unique<ProtectivePutStrategy>(
                symbol, current_price, current_price * 0.95, volatility, risk_free_rate, expiry
            );
            
        case BULL_CALL_SPREAD:
            return std::make_unique<BullCallSpreadStrategy>(
                symbol, current_price, current_price * 0.95, current_price * 1.05, 
                volatility, risk_free_rate, expiry
            );
            
        case BEAR_PUT_SPREAD:
            return std::make_unique<BearPutSpreadStrategy>(
                symbol, current_price, current_price * 1.05, current_price * 0.95, 
                volatility, risk_free_rate, expiry
            );
            
        case STRADDLE:
            return std::make_unique<StraddleStrategy>(
                symbol, current_price, current_price, volatility, risk_free_rate, expiry
            );
            
            
        default:
            throw std::invalid_argument("Strategy type not implemented");
    }
}

#endif // OPTION_STRATEGIES_H