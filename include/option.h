#ifndef OPTION_H
#define OPTION_H

#include <string>
#include <ctime>

enum OptionType {
    CALL,
    PUT
};

class Option {
private:
    std::string symbol;
    OptionType type;
    double strike_price;
    std::time_t expiration_date;
    double current_price;

public:
    Option(const std::string& symbol, OptionType type, double strike, std::time_t expiry);

    // Getters
    std::string getSymbol() const;
    OptionType getType() const;
    double getStrikePrice() const;
    std::time_t getExpirationDate() const;
    double getCurrentPrice() const;

    // Setters
    void setCurrentPrice(double price);
};

#endif // OPTION_H