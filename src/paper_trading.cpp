#include "../include/paper_trading.h"
#include "../include/black_scholes.h"
#include <iostream>
#include <algorithm>
#include <stdexcept>
#include <iomanip>

// Constructor with initial balance and API key
PaperTradingSystem::PaperTradingSystem(double initial_balance, const std::string& alpha_vantage_api_key) 
    : cash_balance(initial_balance), 
      initial_balance(initial_balance),
      market_data(alpha_vantage_api_key)
{
}

bool PaperTradingSystem::buyOption(const Option& option, int quantity) {
    // Validate inputs
    if (quantity <= 0) {
        std::cerr << "Invalid quantity. Must be positive." << std::endl;
        return false;
    }

    // Calculate total cost
    double current_price = option.getCurrentPrice();
    double total_cost = current_price * quantity;
    double transaction_fee = 1.0 * quantity;  // Simple transaction fee model

    // Check if sufficient cash balance
    if (total_cost + transaction_fee > cash_balance) {
        std::cerr << "Insufficient funds to buy option " << option.getSymbol() 
                  << ". Required: $" << total_cost + transaction_fee 
                  << ", Available: $" << cash_balance << std::endl;
        return false;
    }

    // Create position
    OptionPosition position{option, quantity, current_price, std::time(nullptr)};
    open_positions.push_back(position);

    // Update cash balance with cost and transaction fee
    cash_balance -= (total_cost + transaction_fee);

    std::cout << "Bought " << quantity << " " 
              << (option.getType() == CALL ? "Call" : "Put") 
              << " options of " << option.getSymbol() 
              << " at $" << current_price << std::endl;

    return true;
}

bool PaperTradingSystem::sellOption(const Option& option, int quantity) {
    // Find matching positions
    auto pos_it = std::find_if(open_positions.begin(), open_positions.end(), 
        [&option, quantity](const OptionPosition& pos) {
            return pos.option.getSymbol() == option.getSymbol() && 
                   pos.option.getType() == option.getType() && 
                   pos.option.getStrikePrice() == option.getStrikePrice() &&
                   pos.quantity >= quantity;
        });

    // Check if position exists
    if (pos_it == open_positions.end()) {
        std::cerr << "No sufficient position to sell option " << option.getSymbol() << std::endl;
        return false;
    }

    // Calculate selling price and transaction fee
    double current_price = option.getCurrentPrice();
    double total_revenue = current_price * quantity;
    double transaction_fee = 1.0 * quantity;  // Simple transaction fee model

    // Update position
    pos_it->quantity -= quantity;
    if (pos_it->quantity == 0) {
        open_positions.erase(pos_it);
    }

    // Update cash balance
    cash_balance += (total_revenue - transaction_fee);

    std::cout << "Sold " << quantity << " " 
              << (option.getType() == CALL ? "Call" : "Put") 
              << " options of " << option.getSymbol() 
              << " at $" << current_price << std::endl;

    return true;
}

double PaperTradingSystem::calculatePortfolioValue() const {
    double total_value = cash_balance;

    for (const auto& position : open_positions) {
        // Use current market price to value the position
        total_value += position.option.getCurrentPrice() * position.quantity;
    }

    return total_value;
}

void PaperTradingSystem::closeExpiredPositions() {
    std::time_t current_time = std::time(nullptr);

    // Remove expired positions
    auto it = open_positions.begin();
    while (it != open_positions.end()) {
        if (it->option.getExpirationDate() <= current_time) {
            // Automatically cash out expired options
            double current_price = it->option.getCurrentPrice();
            cash_balance += current_price * it->quantity;
            
            std::cout << "Expired Position Closed: " 
                      << it->quantity << " " 
                      << (it->option.getType() == CALL ? "Call" : "Put") 
                      << " options of " << it->option.getSymbol() 
                      << " at $" << current_price << std::endl;
            
            // Remove the position
            it = open_positions.erase(it);
        } else {
            ++it;
        }
    }
}

void PaperTradingSystem::printPortfolio() const {
    std::cout << "===== Paper Trading Portfolio =====" << std::endl;
    std::cout << std::fixed << std::setprecision(2);
    
    // Initial balance information
    std::cout << "Initial Balance: $" << initial_balance << std::endl;
    std::cout << "Current Cash Balance: $" << cash_balance << std::endl;
    
    // Performance metrics
    double current_portfolio_value = calculatePortfolioValue();
    double total_gain_loss = current_portfolio_value - initial_balance;
    double gain_loss_percentage = (total_gain_loss / initial_balance) * 100;
    
    std::cout << "Total Portfolio Value: $" << current_portfolio_value << std::endl;
    std::cout << "Total Gain/Loss: $" << total_gain_loss 
              << " (" << gain_loss_percentage << "%)" << std::endl;
    
    // Open Positions Details
    std::cout << "\nOpen Positions:" << std::endl;
    if (open_positions.empty()) {
        std::cout << "No open positions." << std::endl;
    } else {
        for (const auto& position : open_positions) {
            std::cout << "- " << position.option.getSymbol() 
                      << " " << (position.option.getType() == CALL ? "Call" : "Put")
                      << " Strike: $" << position.option.getStrikePrice()
                      << " Quantity: " << position.quantity
                      << " Entry Price: $" << position.entry_price 
                      << " Current Price: $" << position.option.getCurrentPrice()
                      << std::endl;
        }
    }
    
    std::cout << "=================================" << std::endl;
}

double PaperTradingSystem::getCashBalance() const {
    return cash_balance;
}