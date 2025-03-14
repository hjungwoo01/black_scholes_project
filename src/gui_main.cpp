#include <QApplication>
#include "../include/options_calculator_gui.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    OptionsCalculatorGUI mainWindow;
    mainWindow.show();
    return app.exec();
}