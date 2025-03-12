#include "../include/option.h"

Option::Option(const std::string& symbol, OptionType type, double strike, std::time_t expiry)
    : symbol(symbol), type(type), strike_price(strike), expiration_date(expiry), current_price(0.0) {}

std::string Option::getSymbol() const {
    return symbol;
}

OptionType Option::getType() const {
    return type;
}

double Option::getStrikePrice() const {
    return strike_price;
}

std::time_t Option::getExpirationDate() const {
    return expiration_date;
}

double Option::getCurrentPrice() const {
    return current_price;
}

void Option::setCurrentPrice(double price) {
    current_price = price;
}