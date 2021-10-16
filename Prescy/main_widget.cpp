#include "main_widget.hpp"
#include "indicators_widget.hpp"

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

#include <QDebug>
#include <QDir>

#include <Engine/data_source/stock_data_source.hpp>
#include <Engine/data_source/stock_query.hpp>
#include <Engine/exception.hpp>

prescybase::MainWidget::MainWidget(QWidget* parent) :
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

    auto addStockButton = new QPushButton("Add Stock", this);
    addStockButton->setMaximumWidth(140);
    addStockButton->setStatusTip("Add a new stock");
    connect(addStockButton, &QPushButton::clicked, this, [this]() {
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
                    auto query = prescyengine::StockQuery{tickerData[0].toStdString(), intervalEdit->currentText().toStdString()};
                    addStock(query);
                    _registry.addStockQuery(query);
                } catch (prescyengine::PrescyException& e) {
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

    auto removeStockButton = new QPushButton("Remove Stock", this);
    removeStockButton->setMaximumWidth(140);
    removeStockButton->setStatusTip("Remove the selected stock");
    connect(removeStockButton, &QPushButton::clicked, this, [this]() {
        if (_stocks.currentItem()) {
            try {
                auto entry = qobject_cast<prescybase::StockListEntry*>(_stocks.itemWidget(_stocks.currentItem()));
                auto query = prescyengine::StockQuery(entry->symbol(), entry->range());
                _dataSource.removeQuery(query);
                _registry.removeStockQuery(query);
            } catch (prescyengine::PrescyException& e) {
                qDebug("%s\n", e.what());
            }

            delete _stocks.takeItem(_stocks.currentRow());
        }
    });

    _stocks.setFocusPolicy(Qt::NoFocus);

    auto indicators = std::vector<prescyengine::StockIndicator>{};
    try {
        indicators = _registry.indicators();
    } catch (prescyengine::PrescyException& e) {
        QMessageBox::warning(this, "Error fetching indicators: ", e.what());
    }
    auto indicatorsWidget = new prescybase::IndicatorsWidget(indicators, this);
    connect(indicatorsWidget,
            &IndicatorsWidget::indicatorAddedOrUpdated,
            this,
            [this](const prescyengine::StockIndicator& indicator) {
        for (auto i = 0; i < _stocks.count(); ++i) {
            auto entry = qobject_cast<prescybase::StockListEntry*>(_stocks.itemWidget(_stocks.item(i)));
            entry->addOrUpdateIndicator(indicator);
        }
        try {
            _registry.addOrUpdateIndicator(indicator);
        } catch (prescyengine::PrescyException& e) {
            QMessageBox::warning(this, "Error updating registry: ", e.what());
        }
    });
    connect(indicatorsWidget,
            &IndicatorsWidget::indicatorRemoved,
            this,
            [this](const std::string& name) {
        for (auto i = 0; i < _stocks.count(); ++i) {
            auto entry = qobject_cast<prescybase::StockListEntry*>(_stocks.itemWidget(_stocks.item(i)));
            entry->removeIndicator(name);
        }
        try {
            _registry.removeIndicator(name);
        } catch (prescyengine::PrescyException& e) {
            QMessageBox::warning(this, "Error updating registry: ", e.what());
        }
    });

    connect(&_timer, &QTimer::timeout, this, [this]() {
        if (_lastRefreshed == 5) {
            _lastRefreshedLabel.setText("Querying...");
            _lastRefreshed++;
            try {
                _dataSource.performQueries();
            } catch (prescyengine::PrescyException& e) {
                qDebug("%s\n", e.what());
            }
            refreshStocks();
        } else {
            _lastRefreshed++;
            _lastRefreshedLabel.setText("Last refreshed " + QString::number(_lastRefreshed) + " sec ago");
        }
    });


    auto layout = new QGridLayout{this};
    layout->addWidget(addStockButton, 0, 0, 1, 1);
    layout->addWidget(removeStockButton, 0, 1, 1, 1);
    layout->addWidget(&_lastRefreshedLabel, 0, 2, 1, 1);
    layout->setAlignment(&_lastRefreshedLabel, Qt::AlignRight);
    layout->addWidget(&_stocks, 1, 0, 1, 3);
    layout->addWidget(indicatorsWidget, 2, 0, 1, 3);


    for (const auto& indicator : _registry.indicators()) {
        addIndicator(indicator);
    }

    for (const auto& query : _registry.stockQueries()) {
        addStock(query);
    }

    _timer.start(1000);
}

void prescybase::MainWidget::refreshStocks() {
    _lastRefreshed = 0;
    _lastRefreshedLabel.setText("Refreshing...");
    try {
        for (auto i = 0; i < _stocks.count(); ++i) {
            auto entry = qobject_cast<prescybase::StockListEntry*>(_stocks.itemWidget(_stocks.item(i)));
            auto data = _dataSource.data(prescyengine::StockQuery{entry->symbol(), entry->range()});
            entry->setData(data);
            entry->repaint();
        }
    } catch (prescyengine::PrescyException& e) {
        _lastRefreshedLabel.setText("Unable to refresh.");
        qDebug("%s\n", e.what());
        _timer.stop();
    }
}

void prescybase::MainWidget::addStock(const prescyengine::StockQuery& query) {
    _dataSource.addQuery(query);
    _dataSource.performQueries();

    auto stockListEntry = new StockListEntry(query.symbol,
                                             _companyNames[query.symbol],
                                             query.range,
                                             _registry.indicators(),
                                             this);
    auto item = new QListWidgetItem(&_stocks);
    _stocks.insertItem(_stocks.count(), item);
    _stocks.setItemWidget(item, stockListEntry);
    item->setSizeHint(stockListEntry->sizeHint());
    refreshStocks();
}

void prescybase::MainWidget::addIndicator(const prescyengine::StockIndicator& indicator) {
    for (auto i = 0; i < _stocks.count(); ++i) {
        auto entry = qobject_cast<prescybase::StockListEntry*>(_stocks.itemWidget(_stocks.item(i)));
        entry->addOrUpdateIndicator(indicator);
    }
}
