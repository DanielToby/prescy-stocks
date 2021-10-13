#include "stock_widget.hpp"

#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFile>
#include <QGridLayout>
#include <QLineEdit>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QStandardPaths>
#include <QToolBar>

#include <QDebug>
#include <QDir>

#include <Engine/data_source/stock_data_source.hpp>
#include <Engine/data_source/stock_query.hpp>
#include <Engine/exception.hpp>

prescybase::StockWidget::StockWidget(QWidget* parent) :
    QWidget{parent},
    _registry{QStandardPaths::writableLocation(QStandardPaths::HomeLocation).toStdString() + "/.prescy/registry.json"},
    _dataSource{},
    _stocks{this},
    _timer{this},
    _lastRefreshed{0},
    _lastRefreshedLabel{this},
    _companyNames{},
    _symbolEdit{this} {
    QFile csvFile(":/resources/stocks/nasdaq_screener_1628640750060.csv");
    if (!csvFile.open(QIODevice::ReadOnly)) {
        qDebug("Error reading symbols");
    }
    while (!csvFile.atEnd()) {
        auto wordList = csvFile.readLine().split(',');
        auto symbol = wordList[0].toStdString();
        auto name = wordList[1].toStdString();
        _symbolEdit.addItem(QString::fromStdString(symbol + "(" + name + ")"));
        _companyNames[symbol] = name;
    }
    _symbolEdit.setEditable(true);
    _symbolEdit.setCurrentText("");
    _symbolEdit.setVisible(false);
    _symbolEdit.setMaximumWidth(300);

    QAction* addStockAction = new QAction("Add Stock", this);
    addStockAction->setShortcuts(QKeySequence::New);
    addStockAction->setStatusTip("Add a new stock");
    connect(addStockAction, &QAction::triggered, this, [this]() {
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
        connect(buttonBox, &QDialogButtonBox::accepted, addStockDialog, [this, intervalEdit, addStockDialog]() {
            if (!_symbolEdit.currentText().isEmpty()) {
                try {
                    auto tickerData = _symbolEdit.currentText().split(")").first().split("(");
                    auto query = prescy::StockQuery{tickerData[0].toStdString(), intervalEdit->currentText().toStdString()};
                    addStock(query);
                    _registry.addStockQuery(query);
                } catch (prescy::PrescyException& e) {
                    QMessageBox::warning(this, "Error updating registry: ", e.what());
                }
            }
            _symbolEdit.setVisible(false);
            addStockDialog->close();
        });

        _symbolEdit.setVisible(true);

        auto addStockDialogLayout = new QVBoxLayout(addStockDialog);
        addStockDialogLayout->addWidget(new QLabel("Symbol"));
        addStockDialogLayout->addWidget(&_symbolEdit);
        addStockDialogLayout->addWidget(new QLabel("Interval"));
        addStockDialogLayout->addWidget(intervalEdit);
        addStockDialogLayout->addWidget(buttonBox);
        addStockDialog->setLayout(addStockDialogLayout);

        addStockDialog->show();
    });

    QAction* removeStockAction = new QAction("Remove Stock", this);
    removeStockAction->setShortcuts(QKeySequence::Delete);
    removeStockAction->setStatusTip("Remove the selected stock");
    connect(removeStockAction, &QAction::triggered, this, [this]() {
        if (_stocks.currentItem()) {
            try {
                auto entry = qobject_cast<prescybase::StockListEntry*>(_stocks.itemWidget(_stocks.currentItem()));
                auto query = prescy::StockQuery(entry->symbol(), entry->range());
                _dataSource.removeQuery(query);
                _registry.removeStockQuery(query);
            } catch (prescy::PrescyException& e) {
                qDebug("%s\n", e.what());
            }

            delete _stocks.takeItem(_stocks.currentRow());
        }
    });

    QAction* addIndicatorAction = new QAction("Add Indicator", this);
    addIndicatorAction->setStatusTip("Add a new indicator");
    connect(addIndicatorAction, &QAction::triggered, this, [this]() {
        auto addIndicatorDialog = new QDialog(this);
        addIndicatorDialog->setWindowTitle("Add Indicator");
        auto nameEdit = new QLineEdit(this);
        auto expressionEdit = new QPlainTextEdit(this);

        auto buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok, addIndicatorDialog);
        connect(buttonBox, &QDialogButtonBox::accepted, addIndicatorDialog, [this, nameEdit, expressionEdit, addIndicatorDialog]() {
            if (!nameEdit->text().isEmpty() && !expressionEdit->toPlainText().isEmpty()) {
                try {
                    auto indicator = prescy::StockIndicator(nameEdit->text().toStdString(),
                                                            expressionEdit->toPlainText().toStdString());
                    addIndicator(indicator);
                    _registry.addStockIndicator(indicator);
                } catch (prescy::PrescyException& e) {
                    QMessageBox::warning(this, "Error updating registry: ", e.what());
                }
            }
            addIndicatorDialog->close();
        });

        auto addIndicatorDialogLayout = new QVBoxLayout(addIndicatorDialog);
        addIndicatorDialogLayout->addWidget(new QLabel("Name"));
        addIndicatorDialogLayout->addWidget(nameEdit);
        addIndicatorDialogLayout->addWidget(new QLabel("Expression"));
        auto luaLabel = new QLabel("<a href=\"https://www.lua.org/manual/5.4/\">Lua 5.4 Manual</a>");
        luaLabel->setTextFormat(Qt::RichText);
        luaLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);
        luaLabel->setOpenExternalLinks(true);
        addIndicatorDialogLayout->addWidget(luaLabel);
        addIndicatorDialogLayout->addWidget(expressionEdit);
        addIndicatorDialogLayout->addWidget(buttonBox);
        addIndicatorDialog->setLayout(addIndicatorDialogLayout);

        addIndicatorDialog->setMinimumSize(400, 200);
        addIndicatorDialog->show();
    });

    QAction* removeIndicatorAction = new QAction("Remove Indicator", this);
    removeIndicatorAction->setStatusTip("Remove the selected indicator");
    connect(removeIndicatorAction, &QAction::triggered, this, [this]() {
        auto removeIndicatorDialog = new QDialog(this);
        removeIndicatorDialog->setWindowTitle("Remove Indicator");
        auto nameEdit = new QComboBox(this);
        try {
            auto indicators = _registry.stockIndicators();
            for (const auto& indicator : indicators) {
                nameEdit->addItem(QString::fromStdString(indicator.name));
            }
        } catch (prescy::PrescyException& e) {
            QMessageBox::warning(this, "Error fetching indicators: ", e.what());
        }

        auto buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok, removeIndicatorDialog);
        connect(buttonBox, &QDialogButtonBox::accepted, removeIndicatorDialog, [this, nameEdit, removeIndicatorDialog]() {
            if (!nameEdit->currentText().isEmpty()) {
                try {
                    auto name = nameEdit->currentText().toStdString();
                    for (auto i = 0; i < _stocks.count(); ++i) {
                        auto entry = qobject_cast<prescybase::StockListEntry*>(_stocks.itemWidget(_stocks.item(i)));
                        entry->removeIndicator(name);
                    }
                    _registry.removeStockIndicator(name);
                } catch (prescy::PrescyException& e) {
                    QMessageBox::warning(this, "Error updating registry: ", e.what());
                }
            }
            removeIndicatorDialog->close();
        });

        auto removeStockIndicatorLayout = new QVBoxLayout(removeIndicatorDialog);
        removeStockIndicatorLayout->addWidget(new QLabel("Name"));
        removeStockIndicatorLayout->addWidget(nameEdit);
        removeStockIndicatorLayout->addWidget(buttonBox);
        removeIndicatorDialog->setLayout(removeStockIndicatorLayout);

        removeIndicatorDialog->setMinimumSize(400, 200);
        removeIndicatorDialog->show();
    });

    auto toolBar = new QToolBar(this);
    toolBar->addAction(addStockAction);
    toolBar->addAction(removeStockAction);
    toolBar->addAction(addIndicatorAction);
    toolBar->addAction(removeIndicatorAction);

    connect(&_timer, &QTimer::timeout, this, [this]() {
        if (_lastRefreshed == 5) {
            _lastRefreshedLabel.setText("Querying...");
            _lastRefreshed++;
            try {
                _dataSource.performQueries();
            } catch (prescy::PrescyException& e) {
                qDebug("%s\n", e.what());
            }
        } else if (_lastRefreshed == 10) {
            refreshStocks();
        } else {
            _lastRefreshed++;
            _lastRefreshedLabel.setText("Last refreshed " + QString::number(_lastRefreshed) + " sec ago");
        }
    });

    _stocks.setFocusPolicy(Qt::NoFocus);

    auto layout = new QGridLayout{this};
    layout->addWidget(toolBar, 0, 0, 1, 1);
    layout->addWidget(&_lastRefreshedLabel, 0, 1, 1, 1);
    layout->setAlignment(&_lastRefreshedLabel, Qt::AlignRight);
    layout->addWidget(&_stocks, 1, 0, 1, 2);

    for (const auto& indicator : _registry.stockIndicators()) {
        addIndicator(indicator);
    }

    for (const auto& query : _registry.stockQueries()) {
        addStock(query);
    }

    _timer.start(1000);
}

void prescybase::StockWidget::refreshStocks() {
    _lastRefreshed = 0;
    _lastRefreshedLabel.setText("Refreshing...");
    try {
        for (auto i = 0; i < _stocks.count(); ++i) {
            auto entry = qobject_cast<prescybase::StockListEntry*>(_stocks.itemWidget(_stocks.item(i)));
            auto data = _dataSource.data(prescy::StockQuery{entry->symbol(), entry->range()});
            entry->setData(data);
            entry->repaint();
        }
    } catch (prescy::PrescyException& e) {
        _lastRefreshedLabel.setText("Unable to refresh.");
        qDebug("%s\n", e.what());
        _timer.stop();
    }
}

void prescybase::StockWidget::addStock(const prescy::StockQuery& query) {
    _dataSource.addQuery(query);
    _dataSource.performQueries();

    auto stockListEntry = new StockListEntry(query.symbol,
                                             _companyNames[query.symbol],
                                             query.range,
                                             _registry.stockIndicators(),
                                             this);
    auto item = new QListWidgetItem(&_stocks);
    _stocks.insertItem(_stocks.count(), item);
    _stocks.setItemWidget(item, stockListEntry);
    item->setSizeHint(stockListEntry->sizeHint());
    refreshStocks();
}

void prescybase::StockWidget::addIndicator(const prescy::StockIndicator& indicator) {
    for (auto i = 0; i < _stocks.count(); ++i) {
        auto entry = qobject_cast<prescybase::StockListEntry*>(_stocks.itemWidget(_stocks.item(i)));
        entry->addIndicator(indicator);
    }
}
