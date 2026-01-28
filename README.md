# Black-Scholes Options Pricing and Paper Trading System

A C++17 application for option pricing, strategy analysis, volatility estimation, and Monte Carlo simulation. It includes a **CLI** for paper trading with live market data and a **Qt GUI** (Options Calculator) for interactive analytics.

---

## Options Calculator GUI

The **Options Calculator** (`options_calculator_gui`) is a desktop app with four main tabs. Below is what each one does, with screenshots.

### 1. Option Calculator

Compute Black–Scholes option prices and Greeks from your inputs.

- **Inputs:** Symbol (e.g. AAPL), Spot Price, Strike Price, Risk-Free Rate (%), Volatility (%), Days to Expiry  
- **Outputs:** Call Price, Put Price  
- **Greeks** for both Call and Put: **Delta (Δ), Gamma (Γ), Theta (Θ), Vega (ν), Rho (ρ)**  
- **Calculate** runs the pricing and updates all results.

![Option Calculator](images/ex1.png)

---

### 2. Strategy Analyzer

Analyze multi-leg option strategies and see P/L at expiry.

- **Inputs:** Symbol, Spot Price, Risk-Free Rate (%), Volatility (%), Days to Expiry, **Strategy Type** (e.g. Covered Call, Protective Put, Bull Call Spread, Bear Put Spread, Straddle)  
- **Analyze Strategy** computes metrics and refreshes the chart.  
- **Strategy metrics:** Max Profit, Max Loss, Breakeven(s)  
- **Chart:** Profit/Loss at expiry vs. stock price, with a break-even (zero P/L) line.

![Strategy Analyzer](images/ex2.png)

---

### 3. Volatility Analyzer

Estimate historical volatility and inspect rolling windows.

- **Inputs:** Symbol, **Days Lookback**  
- **Calculate Volatility** produces **Annual Volatility** (e.g. 17.92%)  
- **Table:** Date and Price for the lookback period  
- **Chart:** 10-, 20-, and 30-day rolling historical volatility over time

![Volatility Analyzer](images/ex3.png)

---

### 4. Monte Carlo

Price options by simulation and compare to Black–Scholes.

- **Inputs:** Spot Price, Strike Price, Risk-Free Rate (%), Volatility (%), Days to Expiry, **Simulation Count**  
- **Run Simulation** runs the Monte Carlo pricer and updates results + chart.  
- **Pricing results:** Call and Put prices from **Monte Carlo** and from **Black–Scholes** side by side  
- **Chart:** Sample Monte Carlo price paths (e.g. 10 paths) from spot to expiry

![Monte Carlo](images/ex4.png)

---

## Project Overview

- **Option pricing:** Black–Scholes for European calls/puts (with edge-case handling for T→0 and σ→0).  
- **Market data:** Optional Alpha Vantage integration for current and historical prices (CLI).  
- **Paper trading:** Simulated option positions, cash balance, and portfolio value (CLI).

---

## Architecture and features

### Market data: DataFeedInterface

- **`DataFeedInterface`** (ABC in `include/data_feed_interface.h`) defines: `getCurrentPrice`, `fetchHistoricalPrices`, `getHistoricalPrices`, optional `setCurrentPrice`, `lastError()`, and `isStaleQuote()`.
- **Implementations:**
  - **AlphaVantageFeed** — wraps `AlphaVantageClient`; handles errors and exposes `lastError()`.
  - **PaperFeed** — in-memory only (no network), for paper trading and tests.
- **MarketDataProvider** holds **`std::unique_ptr<DataFeedInterface>`** and delegates all calls to the feed. You can inject any feed (e.g. future Alpaca or Yahoo) by implementing the interface. A backwards-compatible constructor from API key builds `AlphaVantageFeed` internally.

### Monte Carlo engine

- **Parallel pricing:** Simulations are split into chunks and run via **`std::async`** (by hardware concurrency). Results are summed and discounted.
- **Progress callback:** Overloads of `priceCallOption` / `pricePutOption` accept **`MonteCarloProgressCallback`** (`std::function<void(int done, int total)>`). The callback is invoked after each chunk so a UI can show progress. To avoid blocking the Qt event loop, run Monte Carlo in a worker (e.g. `QtConcurrent::run` or `QThread`) and use **`QMetaObject::invokeMethod(..., Qt::QueuedConnection)`** to update a progress bar on the main thread.

### Advanced analytics

- **Greeks:** All five (Delta, Gamma, Theta, Vega, Rho) are implemented in **BlackScholesGreeks** and displayed in the Option Calculator tab.
- **Implied volatility:** **Newton–Raphson** in `BlackScholes::calculateImpliedVolatility`; a **Bisection** fallback runs when N–R fails (e.g. vega too small or no convergence) over vol in [0.0001, 5.0]. Optional next step: add an IV calculator in the Option Calculator tab (market price + Call/Put + “Solve IV” button and label).

### Strategy P&L and charts

- **P&L at expiry:** The Strategy Analyzer tab plots profit/loss vs. stock price using Qt Charts and **`OptionStrategy::calculateProfitLoss`**. Covered Call, Protective Put, Bull/Bear spreads, and Straddle implement it. Optional: add **Volatility Smile** (e.g. new tab with IV vs strike using Qt Charts).

### Code quality (C++17)

- Heavy math types passed by **`const &`** where appropriate; primitives by value.
- **`noexcept`** on `OptionalDouble` and can be extended to other getters/helpers that don’t throw.
- **`std::unique_ptr`** used for the data feed inside `MarketDataProvider`; strategy factory already returns `std::unique_ptr<OptionStrategy>`.

---

### Build and CMake

- **New sources** in the build: `alpha_vantage_feed.cpp`, `paper_feed.cpp`, `monte_carlo.cpp` are included in both `option_trading` and `options_calculator_gui` targets. No new dependencies beyond existing Qt/CURL/json.

### Testing / verification

- **Unit**: Option Calculator and Strategy Analyzer should produce same Black–Scholes and Greeks after refactors.
- **Monte Carlo**: Parallel vs same-seed sequential for stability; progress callback when wired to GUI.
- **Data feed**: Run with PaperFeed (no API key) and AlphaVantageFeed; check `lastError()` and staleness/missing-strike handling in UI or logs.

---

## Build

- **Requirements:** CMake 3.14+, C++17, CURL. Optional: Qt5 (Core, Widgets, Charts) for the GUI; nlohmann/json is fetched by CMake if not installed.  
- From the project root:

  ```bash
  mkdir build && cd build
  cmake ..
  cmake --build .
  ```

- If Qt5 is not found, only the CLI target `option_trading` is built.

## Run

- **CLI (paper trading + live data):**  
  `./option_trading`  
  Uses `ALPHA_VANTAGE_API_KEY` from a `.env` file in the project or next to the executable.

- **GUI (Options Calculator):**  
  `./options_calculator_gui`

## Dependencies

- C++17 compiler  
- CMake 3.14+  
- CURL  
- nlohmann/json (via FetchContent if needed)  
- Qt5 (Core, Widgets, Charts) — only for the GUI  

## License

See repository for license information.
