#include "main_window.hpp"
#include "stock_widget.hpp"

#include <QDir>
#include <QLayout>
#include <QMessageBox>
#include <QStandardPaths>

#include <iostream>
#include <string>
#include <vector>

namespace prescybase {

MainWindow::MainWindow(QWidget* parent) :
    QMainWindow(parent) {
    setWindowTitle("Prescy");
    setMinimumSize(800, 500);

    auto dataDirectory = QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + "/.prescy";
    if (!QDir(dataDirectory).exists()) {
        QDir().mkdir(dataDirectory);
    }

    auto homePage = new prescybase::StockWidget(this);
    setCentralWidget(homePage);
}

MainWindow::~MainWindow() {
}

}
