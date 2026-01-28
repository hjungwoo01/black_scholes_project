#ifndef OPTION_H
#define OPTION_H

#include <string>
#include <ctime>

enum OptionType {
    CALL,
    PUT
};

// Option contract: stores underlying symbol, strike, expiry (time_t), and current option premium.
class Option {
private:
    std::string underlying_symbol;
    OptionType type;
    double strike_price;
    std::time_t expiration_date;
    double current_price;

public:
    Option(const std::string& underlying_symbol, OptionType type, double strike, std::time_t expiry);

    // Getters
    std::string getSymbol() const;  // Underlying ticker symbol
    OptionType getType() const;
    double getStrikePrice() const;
    std::time_t getExpirationDate() const;
    double getCurrentPrice() const;

    // Setters
    void setCurrentPrice(double price);
};

#endif // OPTION_H