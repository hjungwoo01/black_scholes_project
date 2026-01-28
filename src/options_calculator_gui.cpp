#include "../include/options_calculator_gui.h"
#include <QApplication>
#include <QMainWindow>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QComboBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QTableWidget>
#include <QHeaderView>
#include <QMessageBox>
#include <QDateEdit>
#include <QChart>
#include <QChartView>
#include <QLineSeries>
#include <QBarSeries>
#include <QBarSet>
#include <QBarCategoryAxis>
#include <QValueAxis>
#include <QDateTime>

#include "../include/option.h"
#include "../include/black_scholes.h"
#include "../include/black_scholes_greeks.h"
#include "../include/market_data.h"
#include "../include/historical_volatility.h"
#include "../include/monte_carlo.h"
#include "../include/option_strategy.h"

OptionsCalculatorGUI::OptionsCalculatorGUI(QWidget *parent) : QMainWindow(parent) {
        setWindowTitle("Options Calculator");
        resize(1200, 800);
        
        // Set up the main tab widget
        QTabWidget *tabWidget = new QTabWidget();
        
        // Create tabs
        createOptionCalculatorTab(tabWidget);
        createStrategyAnalyzerTab(tabWidget);
        createVolatilityAnalyzerTab(tabWidget);
        createMonteCarloTab(tabWidget);
        
        setCentralWidget(tabWidget);
}

void OptionsCalculatorGUI::calculateOptionPrices() {
        QString symbol = symbolInput->text().trimmed();
        if (symbol.isEmpty()) {
            QMessageBox::warning(this, "Error", "Please enter a stock symbol");
            return;
        }
        
        // Get inputs
        double spotPrice = spotPriceInput->value();
        double strikePrice = strikePriceInput->value();
        double riskFreeRate = riskFreeRateInput->value() / 100.0;
        double volatility = volatilityInput->value() / 100.0;
        int daysToExpiry = daysToExpiryInput->value();
        double timeToExpiry = daysToExpiry / 365.0;
        
        // Calculate option prices using Black-Scholes
        double callPrice = BlackScholes::calculateCallPrice(
            spotPrice, strikePrice, riskFreeRate, volatility, timeToExpiry
        );
        
        double putPrice = BlackScholes::calculatePutPrice(
            spotPrice, strikePrice, riskFreeRate, volatility, timeToExpiry
        );
        
        // Calculate Greeks
        OptionGreeks callGreeks = BlackScholesGreeks::calculateCallGreeks(
            spotPrice, strikePrice, riskFreeRate, volatility, timeToExpiry
        );
        
        OptionGreeks putGreeks = BlackScholesGreeks::calculatePutGreeks(
            spotPrice, strikePrice, riskFreeRate, volatility, timeToExpiry
        );
        
        // Update results
        callPriceOutput->setText(QString::number(callPrice, 'f', 2));
        putPriceOutput->setText(QString::number(putPrice, 'f', 2));
        
        // Update Greeks
        callDeltaOutput->setText(QString::number(callGreeks.delta, 'f', 4));
        callGammaOutput->setText(QString::number(callGreeks.gamma, 'f', 4));
        callThetaOutput->setText(QString::number(callGreeks.theta, 'f', 4));
        callVegaOutput->setText(QString::number(callGreeks.vega, 'f', 4));
        callRhoOutput->setText(QString::number(callGreeks.rho, 'f', 4));
        
        putDeltaOutput->setText(QString::number(putGreeks.delta, 'f', 4));
        putGammaOutput->setText(QString::number(putGreeks.gamma, 'f', 4));
        putThetaOutput->setText(QString::number(putGreeks.theta, 'f', 4));
        putVegaOutput->setText(QString::number(putGreeks.vega, 'f', 4));
        putRhoOutput->setText(QString::number(putGreeks.rho, 'f', 4));
}

void OptionsCalculatorGUI::analyzeStrategy() {
        QString symbol = strategySymbolInput->text().trimmed();
        if (symbol.isEmpty()) {
            QMessageBox::warning(this, "Error", "Please enter a stock symbol");
            return;
        }
        
        // Get inputs
        double spotPrice = strategySpotPriceInput->value();
        double riskFreeRate = strategyRiskFreeRateInput->value() / 100.0;
        double volatility = strategyVolatilityInput->value() / 100.0;
        int daysToExpiry = strategyDaysToExpiryInput->value();
        
        // Create expiry date
        std::time_t now = std::time(nullptr);
        std::tm time_info = *std::localtime(&now);
        time_info.tm_mday += daysToExpiry;
        std::time_t expiry = std::mktime(&time_info);
        
        // Create strategy based on selection
        int strategyIndex = strategyTypeCombo->currentIndex();
        auto strategyType = static_cast<OptionStrategyFactory::StrategyType>(strategyIndex);
        
        try {
            std::unique_ptr<OptionStrategy> strategy = OptionStrategyFactory::createStrategy(
                strategyType, 
                symbol.toStdString(), 
                spotPrice, 
                volatility, 
                riskFreeRate, 
                expiry
            );
            
            // Calculate and display strategy metrics
            double maxProfit = strategy->calculateMaxProfit();
            double maxLoss = strategy->calculateMaxLoss();
            auto breakevens = strategy->calculateBreakevens();
            
            strategyMaxProfitOutput->setText(QString::number(maxProfit, 'f', 2));
            strategyMaxLossOutput->setText(QString::number(maxLoss, 'f', 2));
            
            QString breakevenStr;
            for (size_t i = 0; i < breakevens.size(); ++i) {
                breakevenStr += QString::number(breakevens[i], 'f', 2);
                if (i < breakevens.size() - 1) {
                    breakevenStr += ", ";
                }
            }
            strategyBreakevenOutput->setText(breakevenStr);
            
            // Calculate profit/loss at various price points for the chart
            updateStrategyChart(strategy.get(), spotPrice, daysToExpiry, riskFreeRate, volatility);
            
        } catch (const std::exception& e) {
            QMessageBox::warning(this, "Error", QString("Strategy calculation failed: %1").arg(e.what()));
        }
}

void OptionsCalculatorGUI::calculateHistoricalVolatility() {
        QString symbol = volSymbolInput->text().trimmed();
        if (symbol.isEmpty()) {
            QMessageBox::warning(this, "Error", "Please enter a stock symbol");
            return;
        }
        
        int daysLookback = volDaysInput->value();
        
        try {
            // Create dummy price data for demo (in real app, you'd fetch this from API)
            std::vector<double> prices;
            double basePrice = 100.0;
            std::srand(static_cast<unsigned int>(std::time(nullptr)));
            
            for (int i = 0; i < daysLookback + 1; ++i) {
                // Generate random daily price changes (between -2% and +2%)
                double change = (static_cast<double>(std::rand()) / RAND_MAX * 4.0 - 2.0) / 100.0;
                basePrice *= (1.0 + change);
                prices.push_back(basePrice);
            }
            
            // Calculate historical volatility
            double annualVolatility = HistoricalVolatility::calculateFromPrices(prices);
            
            // Display result
            volResultOutput->setText(QString::number(annualVolatility * 100.0, 'f', 2) + "%");
            
            // Update history table
            volHistoryTable->setRowCount(std::min(daysLookback, 30)); // Show up to 30 days
            for (int i = 0; i < std::min(daysLookback, 30); ++i) {
                QTableWidgetItem *dateItem = new QTableWidgetItem(
                    QDate::currentDate().addDays(-i).toString("yyyy-MM-dd")
                );
                QTableWidgetItem *priceItem = new QTableWidgetItem(
                    QString::number(prices[i], 'f', 2)
                );
                
                volHistoryTable->setItem(i, 0, dateItem);
                volHistoryTable->setItem(i, 1, priceItem);
            }
            
            // Show running volatility (using different window sizes)
            updateVolatilityChart(prices);
            
        } catch (const std::exception& e) {
            QMessageBox::warning(this, "Error", QString("Volatility calculation failed: %1").arg(e.what()));
        }
}

void OptionsCalculatorGUI::runMonteCarloSimulation() {
        double spotPrice = mcSpotPriceInput->value();
        double strikePrice = mcStrikePriceInput->value();
        double riskFreeRate = mcRiskFreeRateInput->value() / 100.0;
        double volatility = mcVolatilityInput->value() / 100.0;
        int daysToExpiry = mcDaysToExpiryInput->value();
        double timeToExpiry = daysToExpiry / 365.0;
        int numSimulations = mcSimCountInput->value();
        
        try {
            // Price options using Monte Carlo
            double mcCallPrice = MonteCarloOptionPricer::priceCallOption(
                spotPrice, strikePrice, riskFreeRate, volatility, timeToExpiry, numSimulations
            );
            
            double mcPutPrice = MonteCarloOptionPricer::pricePutOption(
                spotPrice, strikePrice, riskFreeRate, volatility, timeToExpiry, numSimulations
            );
            
            // Calculate Black-Scholes prices for comparison
            double bsCallPrice = BlackScholes::calculateCallPrice(
                spotPrice, strikePrice, riskFreeRate, volatility, timeToExpiry
            );
            
            double bsPutPrice = BlackScholes::calculatePutPrice(
                spotPrice, strikePrice, riskFreeRate, volatility, timeToExpiry
            );
            
            // Display results
            mcCallPriceOutput->setText(QString::number(mcCallPrice, 'f', 2));
            mcPutPriceOutput->setText(QString::number(mcPutPrice, 'f', 2));
            
            bsCallPriceOutput->setText(QString::number(bsCallPrice, 'f', 2));
            bsPutPriceOutput->setText(QString::number(bsPutPrice, 'f', 2));
            
            // Generate and display price paths
            updateMonteCarloChart(spotPrice, riskFreeRate, volatility, timeToExpiry);
            
        } catch (const std::exception& e) {
            QMessageBox::warning(this, "Error", QString("Monte Carlo simulation failed: %1").arg(e.what()));
        }
}

void OptionsCalculatorGUI::createOptionCalculatorTab(QTabWidget *tabWidget) {
        QWidget *tab = new QWidget();
        QVBoxLayout *layout = new QVBoxLayout();
        
        // Input group
        QGroupBox *inputGroup = new QGroupBox("Option Parameters");
        QGridLayout *inputLayout = new QGridLayout();
        
        inputLayout->addWidget(new QLabel("Symbol:"), 0, 0);
        symbolInput = new QLineEdit();
        symbolInput->setText("AAPL");
        inputLayout->addWidget(symbolInput, 0, 1);
        
        inputLayout->addWidget(new QLabel("Spot Price:"), 1, 0);
        spotPriceInput = new QDoubleSpinBox();
        spotPriceInput->setRange(0.01, 10000.0);
        spotPriceInput->setValue(100.0);
        spotPriceInput->setDecimals(2);
        inputLayout->addWidget(spotPriceInput, 1, 1);
        
        inputLayout->addWidget(new QLabel("Strike Price:"), 2, 0);
        strikePriceInput = new QDoubleSpinBox();
        strikePriceInput->setRange(0.01, 10000.0);
        strikePriceInput->setValue(100.0);
        strikePriceInput->setDecimals(2);
        inputLayout->addWidget(strikePriceInput, 2, 1);
        
        inputLayout->addWidget(new QLabel("Risk-Free Rate (%):"), 3, 0);
        riskFreeRateInput = new QDoubleSpinBox();
        riskFreeRateInput->setRange(0.0, 20.0);
        riskFreeRateInput->setValue(2.0);
        riskFreeRateInput->setDecimals(2);
        inputLayout->addWidget(riskFreeRateInput, 3, 1);
        
        inputLayout->addWidget(new QLabel("Volatility (%):"), 4, 0);
        volatilityInput = new QDoubleSpinBox();
        volatilityInput->setRange(1.0, 200.0);
        volatilityInput->setValue(30.0);
        volatilityInput->setDecimals(2);
        inputLayout->addWidget(volatilityInput, 4, 1);
        
        inputLayout->addWidget(new QLabel("Days to Expiry:"), 5, 0);
        daysToExpiryInput = new QSpinBox();
        daysToExpiryInput->setRange(1, 1000);
        daysToExpiryInput->setValue(30);
        inputLayout->addWidget(daysToExpiryInput, 5, 1);
        
        QPushButton *calculateButton = new QPushButton("Calculate");
        inputLayout->addWidget(calculateButton, 6, 0, 1, 2);
        connect(calculateButton, &QPushButton::clicked, this, &OptionsCalculatorGUI::calculateOptionPrices);
        
        inputGroup->setLayout(inputLayout);
        layout->addWidget(inputGroup);
        
        // Results group
        QGroupBox *resultsGroup = new QGroupBox("Option Prices");
        QGridLayout *resultsLayout = new QGridLayout();
        
        resultsLayout->addWidget(new QLabel("Call Price:"), 0, 0);
        callPriceOutput = new QLabel("--");
        resultsLayout->addWidget(callPriceOutput, 0, 1);
        
        resultsLayout->addWidget(new QLabel("Put Price:"), 1, 0);
        putPriceOutput = new QLabel("--");
        resultsLayout->addWidget(putPriceOutput, 1, 1);
        
        resultsGroup->setLayout(resultsLayout);
        layout->addWidget(resultsGroup);
        
        // Greeks group
        QGroupBox *greeksGroup = new QGroupBox("Option Greeks");
        QGridLayout *greeksLayout = new QGridLayout();
        
        // Headers
        greeksLayout->addWidget(new QLabel(""), 0, 0);
        greeksLayout->addWidget(new QLabel("Call"), 0, 1);
        greeksLayout->addWidget(new QLabel("Put"), 0, 2);
        
        // Delta
        greeksLayout->addWidget(new QLabel("Delta (Δ):"), 1, 0);
        callDeltaOutput = new QLabel("--");
        putDeltaOutput = new QLabel("--");
        greeksLayout->addWidget(callDeltaOutput, 1, 1);
        greeksLayout->addWidget(putDeltaOutput, 1, 2);
        
        // Gamma
        greeksLayout->addWidget(new QLabel("Gamma (Γ):"), 2, 0);
        callGammaOutput = new QLabel("--");
        putGammaOutput = new QLabel("--");
        greeksLayout->addWidget(callGammaOutput, 2, 1);
        greeksLayout->addWidget(putGammaOutput, 2, 2);
        
        // Theta
        greeksLayout->addWidget(new QLabel("Theta (Θ):"), 3, 0);
        callThetaOutput = new QLabel("--");
        putThetaOutput = new QLabel("--");
        greeksLayout->addWidget(callThetaOutput, 3, 1);
        greeksLayout->addWidget(putThetaOutput, 3, 2);
        
        // Vega
        greeksLayout->addWidget(new QLabel("Vega (ν):"), 4, 0);
        callVegaOutput = new QLabel("--");
        putVegaOutput = new QLabel("--");
        greeksLayout->addWidget(callVegaOutput, 4, 1);
        greeksLayout->addWidget(putVegaOutput, 4, 2);
        
        // Rho
        greeksLayout->addWidget(new QLabel("Rho (ρ):"), 5, 0);
        callRhoOutput = new QLabel("--");
        putRhoOutput = new QLabel("--");
        greeksLayout->addWidget(callRhoOutput, 5, 1);
        greeksLayout->addWidget(putRhoOutput, 5, 2);
        
        greeksGroup->setLayout(greeksLayout);
        layout->addWidget(greeksGroup);
        
        tab->setLayout(layout);
        tabWidget->addTab(tab, "Option Calculator");
    }
    
void OptionsCalculatorGUI::createStrategyAnalyzerTab(QTabWidget *tabWidget) {
        QWidget *tab = new QWidget();
        QVBoxLayout *layout = new QVBoxLayout();
        
        // Top section with inputs and results
        QHBoxLayout *topLayout = new QHBoxLayout();
        
        // Input group
        QGroupBox *inputGroup = new QGroupBox("Strategy Parameters");
        QGridLayout *inputLayout = new QGridLayout();
        
        inputLayout->addWidget(new QLabel("Symbol:"), 0, 0);
        strategySymbolInput = new QLineEdit();
        strategySymbolInput->setText("AAPL");
        inputLayout->addWidget(strategySymbolInput, 0, 1);
        
        inputLayout->addWidget(new QLabel("Spot Price:"), 1, 0);
        strategySpotPriceInput = new QDoubleSpinBox();
        strategySpotPriceInput->setRange(0.01, 10000.0);
        strategySpotPriceInput->setValue(100.0);
        strategySpotPriceInput->setDecimals(2);
        inputLayout->addWidget(strategySpotPriceInput, 1, 1);
        
        inputLayout->addWidget(new QLabel("Risk-Free Rate (%):"), 2, 0);
        strategyRiskFreeRateInput = new QDoubleSpinBox();
        strategyRiskFreeRateInput->setRange(0.0, 20.0);
        strategyRiskFreeRateInput->setValue(2.0);
        strategyRiskFreeRateInput->setDecimals(2);
        inputLayout->addWidget(strategyRiskFreeRateInput, 2, 1);
        
        inputLayout->addWidget(new QLabel("Volatility (%):"), 3, 0);
        strategyVolatilityInput = new QDoubleSpinBox();
        strategyVolatilityInput->setRange(1.0, 200.0);
        strategyVolatilityInput->setValue(30.0);
        strategyVolatilityInput->setDecimals(2);
        inputLayout->addWidget(strategyVolatilityInput, 3, 1);
        
        inputLayout->addWidget(new QLabel("Days to Expiry:"), 4, 0);
        strategyDaysToExpiryInput = new QSpinBox();
        strategyDaysToExpiryInput->setRange(1, 1000);
        strategyDaysToExpiryInput->setValue(30);
        inputLayout->addWidget(strategyDaysToExpiryInput, 4, 1);
        
        inputLayout->addWidget(new QLabel("Strategy Type:"), 5, 0);
        strategyTypeCombo = new QComboBox();
        strategyTypeCombo->addItem("Covered Call");
        strategyTypeCombo->addItem("Protective Put");
        strategyTypeCombo->addItem("Bull Call Spread");
        strategyTypeCombo->addItem("Bear Put Spread");
        strategyTypeCombo->addItem("Straddle");
        inputLayout->addWidget(strategyTypeCombo, 5, 1);
        
        QPushButton *analyzeButton = new QPushButton("Analyze Strategy");
        inputLayout->addWidget(analyzeButton, 6, 0, 1, 2);
        connect(analyzeButton, &QPushButton::clicked, this, &OptionsCalculatorGUI::analyzeStrategy);
        
        inputGroup->setLayout(inputLayout);
        topLayout->addWidget(inputGroup);
        
        // Results group
        QGroupBox *resultsGroup = new QGroupBox("Strategy Metrics");
        QGridLayout *resultsLayout = new QGridLayout();
        
        resultsLayout->addWidget(new QLabel("Max Profit:"), 0, 0);
        strategyMaxProfitOutput = new QLabel("--");
        resultsLayout->addWidget(strategyMaxProfitOutput, 0, 1);
        
        resultsLayout->addWidget(new QLabel("Max Loss:"), 1, 0);
        strategyMaxLossOutput = new QLabel("--");
        resultsLayout->addWidget(strategyMaxLossOutput, 1, 1);
        
        resultsLayout->addWidget(new QLabel("Breakeven(s):"), 2, 0);
        strategyBreakevenOutput = new QLabel("--");
        resultsLayout->addWidget(strategyBreakevenOutput, 2, 1);
        
        resultsGroup->setLayout(resultsLayout);
        topLayout->addWidget(resultsGroup);
        
        layout->addLayout(topLayout);
        
        // Chart for strategy P/L
        strategyChartView = new QtCharts::QChartView();
        strategyChartView->setMinimumHeight(400);
        layout->addWidget(strategyChartView);
        
        tab->setLayout(layout);
        tabWidget->addTab(tab, "Strategy Analyzer");
    }
    
void OptionsCalculatorGUI::createVolatilityAnalyzerTab(QTabWidget *tabWidget) {
        QWidget *tab = new QWidget();
        QVBoxLayout *layout = new QVBoxLayout();
        
        // Input group
        QGroupBox *inputGroup = new QGroupBox("Volatility Analysis");
        QGridLayout *inputLayout = new QGridLayout();
        
        inputLayout->addWidget(new QLabel("Symbol:"), 0, 0);
        volSymbolInput = new QLineEdit();
        volSymbolInput->setText("AAPL");
        inputLayout->addWidget(volSymbolInput, 0, 1);
        
        inputLayout->addWidget(new QLabel("Days Lookback:"), 1, 0);
        volDaysInput = new QSpinBox();
        volDaysInput->setRange(10, 252);
        volDaysInput->setValue(30);
        inputLayout->addWidget(volDaysInput, 1, 1);
        
        QPushButton *calculateVolButton = new QPushButton("Calculate Volatility");
        inputLayout->addWidget(calculateVolButton, 2, 0);
        connect(calculateVolButton, &QPushButton::clicked, this, &OptionsCalculatorGUI::calculateHistoricalVolatility);
        
        inputLayout->addWidget(new QLabel("Annual Volatility:"), 2, 1);
        volResultOutput = new QLabel("--");
        volResultOutput->setStyleSheet("font-weight: bold;");
        inputLayout->addWidget(volResultOutput, 2, 2);
        
        inputGroup->setLayout(inputLayout);
        layout->addWidget(inputGroup);
        
        // Split view for table and chart
        QHBoxLayout *dataLayout = new QHBoxLayout();
        
        // Price history table
        volHistoryTable = new QTableWidget();
        volHistoryTable->setColumnCount(2);
        volHistoryTable->setHorizontalHeaderLabels(QStringList() << "Date" << "Price");
        volHistoryTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        volHistoryTable->setMinimumWidth(300);
        dataLayout->addWidget(volHistoryTable);
        
        // Volatility chart
        volChartView = new QtCharts::QChartView();
        volChartView->setMinimumWidth(500);
        dataLayout->addWidget(volChartView);
        
        layout->addLayout(dataLayout);
        
        tab->setLayout(layout);
        tabWidget->addTab(tab, "Volatility Analyzer");
    }
    
void OptionsCalculatorGUI::createMonteCarloTab(QTabWidget *tabWidget) {
        QWidget *tab = new QWidget();
        QVBoxLayout *layout = new QVBoxLayout();
        
        // Input and results section
        QHBoxLayout *topLayout = new QHBoxLayout();
        
        // Monte Carlo inputs
        QGroupBox *inputGroup = new QGroupBox("Monte Carlo Parameters");
        QGridLayout *inputLayout = new QGridLayout();
        
        inputLayout->addWidget(new QLabel("Spot Price:"), 0, 0);
        mcSpotPriceInput = new QDoubleSpinBox();
        mcSpotPriceInput->setRange(0.01, 10000.0);
        mcSpotPriceInput->setValue(100.0);
        mcSpotPriceInput->setDecimals(2);
        inputLayout->addWidget(mcSpotPriceInput, 0, 1);
        
        inputLayout->addWidget(new QLabel("Strike Price:"), 1, 0);
        mcStrikePriceInput = new QDoubleSpinBox();
        mcStrikePriceInput->setRange(0.01, 10000.0);
        mcStrikePriceInput->setValue(100.0);
        mcStrikePriceInput->setDecimals(2);
        inputLayout->addWidget(mcStrikePriceInput, 1, 1);
        
        inputLayout->addWidget(new QLabel("Risk-Free Rate (%):"), 2, 0);
        mcRiskFreeRateInput = new QDoubleSpinBox();
        mcRiskFreeRateInput->setRange(0.0, 20.0);
        mcRiskFreeRateInput->setValue(2.0);
        mcRiskFreeRateInput->setDecimals(2);
        inputLayout->addWidget(mcRiskFreeRateInput, 2, 1);
        
        inputLayout->addWidget(new QLabel("Volatility (%):"), 3, 0);
        mcVolatilityInput = new QDoubleSpinBox();
        mcVolatilityInput->setRange(1.0, 200.0);
        mcVolatilityInput->setValue(30.0);
        mcVolatilityInput->setDecimals(2);
        inputLayout->addWidget(mcVolatilityInput, 3, 1);
        
        inputLayout->addWidget(new QLabel("Days to Expiry:"), 4, 0);
        mcDaysToExpiryInput = new QSpinBox();
        mcDaysToExpiryInput->setRange(1, 1000);
        mcDaysToExpiryInput->setValue(30);
        inputLayout->addWidget(mcDaysToExpiryInput, 4, 1);
        
        inputLayout->addWidget(new QLabel("Simulation Count:"), 5, 0);
        mcSimCountInput = new QSpinBox();
        mcSimCountInput->setRange(100, 10000);
        mcSimCountInput->setValue(1000);
        mcSimCountInput->setSingleStep(100);
        inputLayout->addWidget(mcSimCountInput, 5, 1);
        
        QPushButton *runSimButton = new QPushButton("Run Simulation");
        inputLayout->addWidget(runSimButton, 6, 0, 1, 2);
        connect(runSimButton, &QPushButton::clicked, this, &OptionsCalculatorGUI::runMonteCarloSimulation);
        
        inputGroup->setLayout(inputLayout);
        topLayout->addWidget(inputGroup);

        // Results group
        QGroupBox *resultsGroup = new QGroupBox("Pricing Results");
        QGridLayout *resultsLayout = new QGridLayout();
        
        resultsLayout->addWidget(new QLabel(""), 0, 0);
        resultsLayout->addWidget(new QLabel("Monte Carlo"), 0, 1);
        resultsLayout->addWidget(new QLabel("Black-Scholes"), 0, 2);
        
        resultsLayout->addWidget(new QLabel("Call Price:"), 1, 0);
        mcCallPriceOutput = new QLabel("--");
        bsCallPriceOutput = new QLabel("--");
        resultsLayout->addWidget(mcCallPriceOutput, 1, 1);
        resultsLayout->addWidget(bsCallPriceOutput, 1, 2);
        
        resultsLayout->addWidget(new QLabel("Put Price:"), 2, 0);
        mcPutPriceOutput = new QLabel("--");
        bsPutPriceOutput = new QLabel("--");
        resultsLayout->addWidget(mcPutPriceOutput, 2, 1);
        resultsLayout->addWidget(bsPutPriceOutput, 2, 2);
        
        resultsGroup->setLayout(resultsLayout);
        topLayout->addWidget(resultsGroup);
        
        layout->addLayout(topLayout);
        
        // Chart for price paths
        mcChartView = new QtCharts::QChartView();
        mcChartView->setMinimumHeight(400);
        layout->addWidget(mcChartView);
        
        tab->setLayout(layout);
        tabWidget->addTab(tab, "Monte Carlo");
    }
    
void OptionsCalculatorGUI::updateStrategyChart(OptionStrategy* strategy, double spotPrice, int daysToExpiry, double riskFreeRate, double volatility) {
        const int numPoints = 100;
        double minPrice = spotPrice * 0.7;
        double maxPrice = spotPrice * 1.3;
        double priceStep = (maxPrice - minPrice) / (numPoints - 1);
        
        QtCharts::QLineSeries *pnlSeries = new QtCharts::QLineSeries();
        pnlSeries->setName("Profit/Loss at Expiry");
        
        // Calculate P/L at expiry for different price points
        for (int i = 0; i < numPoints; ++i) {
            double price = minPrice + i * priceStep;
            double pnl = strategy->calculateProfitLoss(price);
            pnlSeries->append(price, pnl);
        }
        
        // Set up chart
        QtCharts::QChart *chart = new QtCharts::QChart();
        chart->addSeries(pnlSeries);
        chart->setTitle("Strategy Profit/Loss at Expiry");
        
        QtCharts::QValueAxis *axisX = new QtCharts::QValueAxis();
        axisX->setTitleText("Stock Price");
        axisX->setRange(minPrice, maxPrice);
        
        QtCharts::QValueAxis *axisY = new QtCharts::QValueAxis();
        axisY->setTitleText("Profit/Loss ($)");
        
        chart->addAxis(axisX, Qt::AlignBottom);
        pnlSeries->attachAxis(axisX);
        
        chart->addAxis(axisY, Qt::AlignLeft);
        pnlSeries->attachAxis(axisY);
        
        // Add zero line
        QtCharts::QLineSeries *zeroLine = new QtCharts::QLineSeries();
        zeroLine->setName("Break-Even");
        zeroLine->append(minPrice, 0);
        zeroLine->append(maxPrice, 0);
        QPen zeroPen(Qt::DashLine);
        zeroPen.setColor(Qt::gray);
        zeroLine->setPen(zeroPen);
        chart->addSeries(zeroLine);
        zeroLine->attachAxis(axisX);
        zeroLine->attachAxis(axisY);
        
        strategyChartView->setChart(chart);
        strategyChartView->setRenderHint(QPainter::Antialiasing);
    }
    
void OptionsCalculatorGUI::updateVolatilityChart(const std::vector<double>& prices) {
        QtCharts::QChart *chart = new QtCharts::QChart();
        chart->setTitle("Historical Volatility Analysis");
        
        // Create multiple series with different window sizes
        std::vector<int> windows = {10, 20, 30};
        QStringList categories;
        
        for (int window : windows) {
            if (prices.size() <= window) continue;
            
            QtCharts::QLineSeries *series = new QtCharts::QLineSeries();
            series->setName(QString("%1-Day Volatility").arg(window));
            
            for (size_t i = window; i < prices.size(); ++i) {
                std::vector<double> windowPrices(prices.begin() + (i - window), prices.begin() + i + 1);
                double vol = HistoricalVolatility::calculateFromPrices(windowPrices) * 100.0;
                
                int dayIndex = static_cast<int>(prices.size() - i - 1);
                series->append(dayIndex, vol);
                
                if (window == windows[0]) {
                    categories << QDate::currentDate().addDays(-dayIndex).toString("MM/dd");
                }
            }
            
            chart->addSeries(series);
        }
        
        QtCharts::QValueAxis *axisY = new QtCharts::QValueAxis();
        axisY->setTitleText("Volatility (%)");
        chart->addAxis(axisY, Qt::AlignLeft);
        
        QtCharts::QBarCategoryAxis *axisX = new QtCharts::QBarCategoryAxis();
        axisX->setTitleText("Date");
        axisX->append(categories);
        chart->addAxis(axisX, Qt::AlignBottom);
        
        // Attach axes to series
        for (auto series : chart->series()) {
            series->attachAxis(axisX);
            series->attachAxis(axisY);
        }
        
        volChartView->setChart(chart);
        volChartView->setRenderHint(QPainter::Antialiasing);
}

void OptionsCalculatorGUI::updateMonteCarloChart(double spotPrice, double riskFreeRate, double volatility, double timeToExpiry) {
        QtCharts::QChart *chart = new QtCharts::QChart();
        chart->setTitle("Monte Carlo Price Paths");
        
        // Generate a few price paths for visualization
        const int numPaths = 10;
        const int stepsPerPath = 100;
        double dt = timeToExpiry / stepsPerPath;
        
        for (int path = 0; path < numPaths; ++path) {
            QtCharts::QLineSeries *series = new QtCharts::QLineSeries();
            series->setName(QString("Path %1").arg(path + 1));
            
            double currentPrice = spotPrice;
            series->append(0, currentPrice);
            
            for (int step = 1; step <= stepsPerPath; ++step) {
                // Generate random normal value
                double z = generateNormalRandom();
                
                // GBM equation
                double drift = (riskFreeRate - 0.5 * volatility * volatility) * dt;
                double diffusion = volatility * std::sqrt(dt) * z;
                currentPrice = currentPrice * std::exp(drift + diffusion);
                series->append(step * (timeToExpiry * 365) / stepsPerPath, currentPrice);
            }
            
            chart->addSeries(series);
        }
        
        // Add axes
        QtCharts::QValueAxis *axisX = new QtCharts::QValueAxis();
        axisX->setTitleText("Days");
        axisX->setRange(0, timeToExpiry * 365);
        chart->addAxis(axisX, Qt::AlignBottom);
        
        QtCharts::QValueAxis *axisY = new QtCharts::QValueAxis();
        axisY->setTitleText("Stock Price");
        chart->addAxis(axisY, Qt::AlignLeft);
        
        // Attach axes to series
        for (auto series : chart->series()) {
            series->attachAxis(axisX);
            series->attachAxis(axisY);
        }
        
        mcChartView->setChart(chart);
        mcChartView->setRenderHint(QPainter::Antialiasing);
}

double OptionsCalculatorGUI::generateNormalRandom() {
    // Box-Muller transform to generate normal random numbers
    double u1 = static_cast<double>(rand()) / RAND_MAX;
    double u2 = static_cast<double>(rand()) / RAND_MAX;
    return std::sqrt(-2.0 * std::log(u1)) * std::cos(2.0 * M_PI * u2);
}
