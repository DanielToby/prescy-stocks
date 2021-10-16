#include "main_window.hpp"
#include "main_widget.hpp"

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
    setMinimumSize(800, 600);

    auto dataDirectory = QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + "/.prescy";
    if (!QDir(dataDirectory).exists()) {
        QDir().mkdir(dataDirectory);
    }

    auto mainWidget = new prescybase::MainWidget(this);
    setCentralWidget(mainWidget);
}

MainWindow::~MainWindow() {
}

}
