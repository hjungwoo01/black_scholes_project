# Black-Scholes Options Pricing and Paper Trading System

## Project Overview
This C++ project implements a Black-Scholes options pricing model with a paper trading system. The application provides functionality for:
- Option pricing using the Black-Scholes model
- Market data simulation
- Paper trading (simulated trading without real money)

## Features
- Black-Scholes option pricing for Call and Put options
- Market data provider with historical and current price tracking
- Paper trading system for tracking option positions

## Build Instructions
1. Ensure you have CMake installed (version 3.10 or higher)
2. Clone the repository
3. Create a build directory:
   ```
   mkdir build
   cd build
   ```
4. Generate build files:
   ```
   cmake ..
   ```
5. Build the project:
   ```
   make
   ```

## Running the Application
```
./option_trading
```

## Dependencies
- C++17 compatible compiler
- CMake 3.10+

## Future Improvements
- Real-time market data integration
- More sophisticated implied volatility algorithms
- Advanced portfolio risk management
- Realistic transaction cost modeling