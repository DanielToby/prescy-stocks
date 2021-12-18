#include "main_widget.hpp"
#include "indicator_thumbnail.hpp"

#include <QComboBox>
#include <QDebug>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDir>
#include <QFile>
#include <QGridLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QStandardPaths>
#include <QTimer>

#include <Engine/data_source/stock_data_source.hpp>
#include <Engine/data_source/stock_query.hpp>
#include <Engine/evaluator.hpp>
#include <Engine/exception.hpp>

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
    connect(addStockButton, &QPushButton::clicked, this, [addStockButton, symbolEdit, this]() {
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
        connect(buttonBox, &QDialogButtonBox::accepted, addStockDialog, [addStockButton, intervalEdit, addStockDialog, symbolEdit, this]() {
            if (!symbolEdit->currentText().isEmpty()) {
                auto tickerData = symbolEdit->currentText().split(") ").first().split(" (");
                auto query = prescyengine::StockQuery{tickerData[0].toStdString(), intervalEdit->currentText().toStdString()};
                try {
                    _dataSource.addQuery(query);
                    _registry.addStockQuery(query);
                    rebuildTable();
                } catch (prescyengine::PrescyException& e) {
                    QMessageBox::warning(this, "Error updating registry: ", e.what());
                }
            }
            symbolEdit->setVisible(false);
            addStockDialog->close();
            addStockButton->setEnabled(true);
        });
        connect(buttonBox, &QDialogButtonBox::rejected, addStockDialog, [addStockButton]() {
            addStockButton->setEnabled(true);
        });

        symbolEdit->setVisible(true);

        auto addStockDialogLayout = new QVBoxLayout(addStockDialog);
        addStockDialogLayout->addWidget(new QLabel("Symbol"));
        addStockDialogLayout->addWidget(symbolEdit);
        addStockDialogLayout->addWidget(new QLabel("Interval"));
        addStockDialogLayout->addWidget(intervalEdit);
        addStockDialogLayout->addWidget(buttonBox);
        addStockDialog->setLayout(addStockDialogLayout);

        addStockButton->setEnabled(false);
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
                rebuildTable();
            } catch (prescyengine::PrescyException& e) {
                qDebug("%s\n", e.what());
            }
        }
    });

    auto addIndicatorButton = new QPushButton("Add Indicator", this);
    addIndicatorButton->setMaximumWidth(140);
    addIndicatorButton->setStatusTip("Add a new indicator");
    connect(addIndicatorButton, &QPushButton::clicked, this, [addIndicatorButton, this]() {
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
        connect(buttonBox, &QDialogButtonBox::accepted, page, [this, addIndicatorButton, nameEdit, expressionEdit, page]() {
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
                    } else {
                        _registry.addIndicator(indicator);
                    }
                    rebuildTable();
                    page->close();
                    addIndicatorButton->setEnabled(true);
                } catch (prescyengine::PrescyException& e) {
                    QMessageBox::warning(this, "Error updating registry: ", e.what());
                }
            }
        });
        connect(buttonBox, &QDialogButtonBox::rejected, page, [addIndicatorButton]() {
            addIndicatorButton->setEnabled(true);
        });
        pageLayout->addWidget(buttonBox);
        page->setLayout(pageLayout);
        addIndicatorButton->setEnabled(false);
        page->show();
    });

    auto removeIndicatorButton = new QPushButton("Remove Indicator", this);
    removeIndicatorButton->setMaximumWidth(140);
    removeIndicatorButton->setStatusTip("Remove the selected indicator");
    connect(removeIndicatorButton, &QPushButton::clicked, this, [this]() {
        if (_stocksTable.currentColumn() > 0) {
            try {
                _registry.removeIndicator(_stocksTable.horizontalHeaderItem(_stocksTable.currentColumn())->text().toStdString());
                rebuildTable();
            } catch (prescyengine::PrescyException& e) {
                QMessageBox::warning(this, "Error updating registry: ", e.what());
            }
        }
    });

    auto lastRefreshedLabel = new QLabel(this);
    auto timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, [lastRefreshedLabel, this]() {
        if (_elapsedTime == 5) {
            lastRefreshedLabel->setText("Querying...");
             performQueries();
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

    _stocksTable.verticalHeader()->hide();
    _stocksTable.setShowGrid(false);
    _stocksTable.setFocusPolicy(Qt::NoFocus);

    rebuildTable();

    try {
        for (const auto& query : _registry.stockQueries()) {
            _dataSource.addQuery(query);
        }
    } catch (prescyengine::PrescyException& e) {
        QMessageBox::warning(this, "Error reading registry: ", e.what());
    }

    timer->start(1000);
}

void MainWidget::rebuildTable() {
    try {
        _stocksTable.clear();

        _stocksTable.setRowCount(static_cast<int>(_registry.stockQueries().size()));
        _stocksTable.setColumnCount(static_cast<int>(_registry.indicators().size() + 1));

        _stocksTable.setHorizontalHeaderItem(0, new QTableWidgetItem("Stock"));
        _stocksTable.horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);

        auto i = 0;
        for (const auto& query : _registry.stockQueries()) {
            _stocksTable.setRowHeight(i, 100);
            _stocksTable.setCellWidget(i, 0, new StockListEntry(query.symbol, _companyNames[query.symbol], query.range, this));
            auto j = 0;
            for (const auto& indicator : _registry.indicators()) {
                if (i == 0) {
                    _stocksTable.setHorizontalHeaderItem(j + 1, new QTableWidgetItem(QString::fromStdString(indicator.name)));
                    _stocksTable.horizontalHeader()->setSectionResizeMode(j + 1, QHeaderView::Fixed);
                }
                _stocksTable.setCellWidget(i, j + 1, new IndicatorThumbnail(indicator.name, indicator.expression, indicator.threshold, this));
                ++j;
            }
            ++i;
        }
    } catch (prescyengine::PrescyException& e) {
        QMessageBox::warning(this, "Error reading registry: ", e.what());
    }
    performQueries();
}

void MainWidget::performQueries() {
    try {
        _dataSource.performQueries();
        for (auto i = 0; i < _stocksTable.rowCount(); ++i) {
            auto entry = qobject_cast<StockListEntry*>(_stocksTable.cellWidget(i, 0));
            auto data = _dataSource.data(prescyengine::StockQuery{entry->symbol(), entry->range()});
            entry->setData(data);
            entry->repaint();
            for (auto j = 1; j < _stocksTable.columnCount(); ++j) {
                auto expression =  qobject_cast<IndicatorThumbnail*>(_stocksTable.cellWidget(i, j))->expression();
                auto result = prescyengine::evaluateExpression(data, expression);
                qobject_cast<IndicatorThumbnail*>(_stocksTable.cellWidget(i, j))->setValue(result);
            }
        }
    } catch (prescyengine::PrescyException& e) {
        qDebug("%s\n", e.what());
    }
    _elapsedTime = 0;
}

}
