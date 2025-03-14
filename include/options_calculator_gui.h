#ifndef OPTIONS_CALCULATOR_GUI_H
#define OPTIONS_CALCULATOR_GUI_H

#include <QMainWindow>
#include <QTabWidget>
#include <QLineEdit>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QComboBox>
#include <QLabel>
#include <QTableWidget>
#include <QChart>
#include <QChartView>

#include "option_strategy.h"

QT_BEGIN_NAMESPACE
namespace QtCharts {
    class QChartView;
}
QT_END_NAMESPACE

class OptionsCalculatorGUI : public QMainWindow {
    Q_OBJECT

public:
    OptionsCalculatorGUI(QWidget *parent = nullptr);

private slots:
    void calculateOptionPrices();
    void analyzeStrategy();
    void calculateHistoricalVolatility();
    void runMonteCarloSimulation();

private:
    // Your private member declarations here (same as in your cpp file)
    QLineEdit *symbolInput;
    QDoubleSpinBox *spotPriceInput;
    QDoubleSpinBox *strikePriceInput;
    QDoubleSpinBox *riskFreeRateInput;
    QDoubleSpinBox *volatilityInput;
    QSpinBox *daysToExpiryInput;
    QLabel *callPriceOutput;
    QLabel *putPriceOutput;
    QLabel *callDeltaOutput;
    QLabel *callGammaOutput;
    QLabel *callThetaOutput;
    QLabel *callVegaOutput;
    QLabel *callRhoOutput;
    QLabel *putDeltaOutput;
    QLabel *putGammaOutput;
    QLabel *putThetaOutput;
    QLabel *putVegaOutput;
    QLabel *putRhoOutput;
    
    // Strategy Analyzer Tab UI elements
    QLineEdit *strategySymbolInput;
    QDoubleSpinBox *strategySpotPriceInput;
    QDoubleSpinBox *strategyRiskFreeRateInput;
    QDoubleSpinBox *strategyVolatilityInput;
    QSpinBox *strategyDaysToExpiryInput;
    QComboBox *strategyTypeCombo;
    QLabel *strategyMaxProfitOutput;
    QLabel *strategyMaxLossOutput;
    QLabel *strategyBreakevenOutput;
    QtCharts::QChartView *strategyChartView;
    
    // Volatility Analyzer Tab UI elements
    QLineEdit *volSymbolInput;
    QSpinBox *volDaysInput;
    QLabel *volResultOutput;
    QTableWidget *volHistoryTable;
    QtCharts::QChartView *volChartView;
    
    // Monte Carlo Tab UI elements
    QDoubleSpinBox *mcSpotPriceInput;
    QDoubleSpinBox *mcStrikePriceInput;
    QDoubleSpinBox *mcRiskFreeRateInput;
    QDoubleSpinBox *mcVolatilityInput;
    QSpinBox *mcDaysToExpiryInput;
    QSpinBox *mcSimCountInput;
    QLabel *mcCallPriceOutput;
    QLabel *mcPutPriceOutput;
    QLabel *bsCallPriceOutput;
    QLabel *bsPutPriceOutput;
    QtCharts::QChartView *mcChartView;
    
    void createOptionCalculatorTab(QTabWidget *tabWidget);
    void createStrategyAnalyzerTab(QTabWidget *tabWidget);
    void createVolatilityAnalyzerTab(QTabWidget *tabWidget);
    void createMonteCarloTab(QTabWidget *tabWidget);
    void updateStrategyChart(OptionStrategy* strategy, double spotPrice, int daysToExpiry, double riskFreeRate, double volatility);
    void updateVolatilityChart(const std::vector<double>& prices);
    void updateMonteCarloChart(double spotPrice, double riskFreeRate, double volatility, double timeToExpiry);
    double generateNormalRandom();
};

#endif // OPTIONS_CALCULATOR_GUI_H