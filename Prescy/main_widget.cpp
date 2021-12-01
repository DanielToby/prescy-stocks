#include "main_widget.hpp"
#include "indicator_thumbnail.hpp"

#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QHeaderView>
#include <QGridLayout>
#include <QMessageBox>
#include <QPushButton>
#include <QPlainTextEdit>
#include <QStandardPaths>
#include <QLabel>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QListWidget>
#include <QLineEdit>
#include <QTimer>

#include <Engine/data_source/stock_data_source.hpp>
#include <Engine/data_source/stock_query.hpp>
#include <Engine/exception.hpp>
#include <Engine/evaluator.hpp>

#include <fmt/format.h>

namespace prescybase {

MainWidget::MainWidget(QWidget* parent) :
    QWidget{parent},
    _stocksTable{this},
    _elapsedTime{5},
    _companyNames{},
    _registry{QStandardPaths::writableLocation(QStandardPaths::HomeLocation).toStdString() + "/.prescy/registry.json"},
    _dataSource{} {
    auto symbolEdit = new QComboBox(this);
    auto csvFile = QFile(":/resources/stocks/nasdaq_screener_1628640750060.csv");
    if (!csvFile.open(QIODevice::ReadOnly)) {
        qDebug("Error reading symbols");
    }
    while (!csvFile.atEnd()) {
        auto wordList = csvFile.readLine().split(',');
        auto symbol = wordList[0].toStdString();
        auto name = wordList[1].toStdString();
        symbolEdit->addItem(QString::fromStdString(fmt::format("{} ({})", symbol, name)));
        _companyNames[symbol] = name;
    }
    symbolEdit->setEditable(true);
    symbolEdit->setCurrentText("");
    symbolEdit->setVisible(false);
    symbolEdit->setMaximumWidth(300);

    auto addStockButton = new QPushButton("Add Stock", this);
    addStockButton->setMaximumWidth(140);
    addStockButton->setStatusTip("Add a new stock");
    connect(addStockButton, &QPushButton::clicked, this, [symbolEdit, this]() {
        auto addStockDialog = new QDialog(this);
        addStockDialog->setWindowTitle("Add Stock");

        auto intervalEdit = new QComboBox{addStockDialog};
        intervalEdit->addItem("1w");
        intervalEdit->addItem("1mo");
        intervalEdit->addItem("3mo");
        intervalEdit->addItem("1yr");
        intervalEdit->addItem("5yr");
        intervalEdit->setCurrentIndex(0);

        auto buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok, addStockDialog);
        connect(buttonBox, &QDialogButtonBox::accepted, addStockDialog, [intervalEdit, addStockDialog, symbolEdit, this]() {
            if (!symbolEdit->currentText().isEmpty()) {
                auto tickerData = symbolEdit->currentText().split(") ").first().split(" (");
                auto query = prescyengine::StockQuery{tickerData[0].toStdString(), intervalEdit->currentText().toStdString()};
                addStockToTable(query);

                try {
                    _dataSource.addQuery(query);
                    _registry.addStockQuery(query);
                } catch (prescyengine::PrescyException& e) {
                    QMessageBox::warning(this, "Error updating registry: ", e.what());
                }
            }
            symbolEdit->setVisible(false);
            addStockDialog->close();
        });

        symbolEdit->setVisible(true);

        auto addStockDialogLayout = new QVBoxLayout(addStockDialog);
        addStockDialogLayout->addWidget(new QLabel("Symbol"));
        addStockDialogLayout->addWidget(symbolEdit);
        addStockDialogLayout->addWidget(new QLabel("Interval"));
        addStockDialogLayout->addWidget(intervalEdit);
        addStockDialogLayout->addWidget(buttonBox);
        addStockDialog->setLayout(addStockDialogLayout);

        addStockDialog->show();
    });

    auto removeStockButton = new QPushButton("Remove Stock", this);
    removeStockButton->setMaximumWidth(140);
    removeStockButton->setStatusTip("Remove the selected stock");
    connect(removeStockButton, &QPushButton::clicked, this, [this]() {
        if (_stocksTable.currentRow() != -1 && _stocksTable.currentColumn() == 0) {
            auto entry = qobject_cast<StockListEntry*>(_stocksTable.cellWidget(_stocksTable.currentRow(), 0));
            auto query = prescyengine::StockQuery(entry->symbol(), entry->range());
            try {
                _dataSource.removeQuery(query);
                _registry.removeStockQuery(query);
            } catch (prescyengine::PrescyException& e) {
                qDebug("%s\n", e.what());
            }
            _stocksTable.removeRow(_stocksTable.currentRow());
        }
    });

    auto addIndicatorButton = new QPushButton("Add Indicator", this);
    addIndicatorButton->setMaximumWidth(140);
    addIndicatorButton->setStatusTip("Add a new indicator");
    connect(addIndicatorButton, &QPushButton::clicked, this, [this]() {
        auto page = new QDialog(this);
        auto pageLayout = new QVBoxLayout(page);
        pageLayout->addWidget(new QLabel("Name"));
        auto nameEdit = new QLineEdit(page);
        pageLayout->addWidget(nameEdit);

        auto luaLabel = new QLabel("<a href=\"https://www.lua.org/manual/5.4/\">Lua 5.4 Manual</a>");
        luaLabel->setTextFormat(Qt::RichText);
        luaLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);
        luaLabel->setOpenExternalLinks(true);
        pageLayout->addWidget(luaLabel);

        pageLayout->addWidget(new QLabel("Expression"));
        auto expressionEdit = new QPlainTextEdit(page);
        pageLayout->addWidget(expressionEdit);

        auto buttonBox = new QDialogButtonBox(QDialogButtonBox::Save, page);
        connect(buttonBox, &QDialogButtonBox::accepted, page, [this, nameEdit, expressionEdit, page]() {
            if (!nameEdit->text().isEmpty() && !expressionEdit->toPlainText().isEmpty()) {
                try {
                    auto indicator = prescyengine::StockIndicator(nameEdit->text().toStdString(), expressionEdit->toPlainText().toStdString());
                    auto indicators = _registry.indicators();
                    if (std::any_of(std::begin(indicators),
                                    std::end(indicators),
                                    [nameEdit](const prescyengine::StockIndicator& indicator) {
                                        return indicator.name == nameEdit->text().toStdString();
                                    })) {
                        QMessageBox::warning(this, "Invalid name", "Please use a unique indicator name.");
                        _registry.updateIndicator(indicator);
                        updateIndicatorInTable(indicator);
                    } else {
                        _registry.addIndicator(indicator);
                        addIndicatorToTable(indicator);
                    }
                    page->close();
                } catch (prescyengine::PrescyException& e) {
                    QMessageBox::warning(this, "Error updating registry: ", e.what());
                }
            }
        });
        pageLayout->addWidget(buttonBox);
        page->setLayout(pageLayout);
        page->show();
    });

    auto removeIndicatorButton = new QPushButton("Remove Indicator", this);
    removeIndicatorButton->setMaximumWidth(140);
    removeIndicatorButton->setStatusTip("Remove the selected indicator");
    connect(removeIndicatorButton, &QPushButton::clicked, this, [this]() {
        if (_stocksTable.currentColumn() > 0) {
            try {
                _registry.removeIndicator(_stocksTable.horizontalHeaderItem(_stocksTable.currentColumn())->text().toStdString());
            } catch (prescyengine::PrescyException& e) {
                QMessageBox::warning(this, "Error updating registry: ", e.what());
            }
            _stocksTable.removeColumn(_stocksTable.currentColumn());
            _stocksTable.horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
        }
    });

    auto lastRefreshedLabel = new QLabel(this);
    auto timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, [lastRefreshedLabel, this]() {
        if (_elapsedTime == 5) {
            lastRefreshedLabel->setText("Querying...");
            refresh();
            _elapsedTime = 0;
        }
        _elapsedTime++;
        lastRefreshedLabel->setText(QString::fromStdString(fmt::format("Refreshed {}s ago", _elapsedTime)));
    });

    auto layout = new QGridLayout{this};
    layout->addWidget(addStockButton, 0, 0, 1, 1);
    layout->addWidget(removeStockButton, 0, 1, 1, 1);
    layout->addWidget(addIndicatorButton, 0, 2, 1, 1);
    layout->addWidget(removeIndicatorButton, 0, 3, 1, 1);
    layout->addWidget(lastRefreshedLabel, 0, 4, 1, 1);
    layout->setAlignment(lastRefreshedLabel, Qt::AlignRight);
    layout->addWidget(&_stocksTable, 1, 0, -1, -1);

    _stocksTable.setColumnCount(1);
    _stocksTable.setHorizontalHeaderLabels(QStringList{"Stock Information"});
    _stocksTable.horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    _stocksTable.verticalHeader()->hide();
    _stocksTable.setShowGrid(false);
    _stocksTable.setFocusPolicy(Qt::NoFocus);

    try {
        for (const auto& query : _registry.stockQueries()) {
            addStockToTable(query);
            _dataSource.addQuery(query);
        }
        for (const auto& indicator : _registry.indicators()) {
            addIndicatorToTable(indicator);
        }
    } catch (prescyengine::PrescyException& e) {
        QMessageBox::warning(this, "Error reading registry: ", e.what());
    }

    timer->start(1000);
}

void MainWidget::addStockToTable(const prescyengine::StockQuery& query) {
    auto stockListEntry = new StockListEntry(query.symbol, _companyNames[query.symbol], query.range, this);
    _stocksTable.insertRow(_stocksTable.rowCount());
    _stocksTable.setRowHeight(_stocksTable.rowCount() - 1, 100);
    _stocksTable.setCellWidget(_stocksTable.rowCount() - 1, 0, stockListEntry);
}

void MainWidget::addIndicatorToTable(const prescyengine::StockIndicator& indicator) {
    _stocksTable.insertColumn(_stocksTable.columnCount());
    _stocksTable.setHorizontalHeaderItem(_stocksTable.columnCount() - 1, new QTableWidgetItem(QString::fromStdString(indicator.name)));
    _stocksTable.horizontalHeader()->setSectionResizeMode(_stocksTable.columnCount() - 1, QHeaderView::Fixed);

    for (auto row = 0; row < _stocksTable.rowCount(); ++row) {
        auto indicatorEntry = new IndicatorThumbnail(indicator.name, indicator.threshold, this);
        _stocksTable.setCellWidget(row, _stocksTable.columnCount() - 1, indicatorEntry);
    }

    _stocksTable.resizeColumnsToContents();
    _stocksTable.horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
}

void MainWidget::updateIndicatorInTable(const prescyengine::StockIndicator &indicator) {
    for (auto col = 1; col < _stocksTable.columnCount(); ++col) {
        if (_stocksTable.horizontalHeaderItem(col)->text().toStdString() == indicator.name) {
            for (auto row = 0; row < _stocksTable.rowCount(); ++row) {
                auto thumbnail = qobject_cast<IndicatorThumbnail*>(_stocksTable.cellWidget(row, col));
                thumbnail->setThreshold(indicator.threshold);
            }
        }
    }
}

void MainWidget::refresh() {
    _dataSource.performQueries();
    auto indicators = _registry.indicators();
    for (auto i = 0; i < _stocksTable.rowCount(); ++i) {
        auto entry = qobject_cast<StockListEntry*>(_stocksTable.cellWidget(i, 0));
        try {
            auto data = _dataSource.data(prescyengine::StockQuery{entry->symbol(), entry->range()});
            entry->setData(data);
            entry->repaint();
            for (auto j = 0; j < _stocksTable.columnCount() - 1; ++j) {
                auto result = prescyengine::evaluateExpression(data, indicators[j].expression);
                auto thumbnail = qobject_cast<IndicatorThumbnail*>(_stocksTable.cellWidget(i, j + 1));
                thumbnail->setValue(result);
            }
        } catch (prescyengine::PrescyException& e) {
            qDebug("%s\n", e.what());
        }
    }
}

}
