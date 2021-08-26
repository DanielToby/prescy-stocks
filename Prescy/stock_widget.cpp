#include "stock_widget.hpp"

#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFile>
#include <QGridLayout>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QStandardPaths>

#include <QDebug>
#include <QDir>

#include <Engine/data_source/stock_data_source.hpp>
#include <Engine/data_source/stock_query.hpp>
#include <Engine/exception.hpp>

prescybase::StockWidget::StockWidget(QWidget* parent) :
    QWidget{parent},
    _registry{QStandardPaths::writableLocation(QStandardPaths::HomeLocation).toStdString() + "/.prescy/registry.json"},
    _dataSource{QStandardPaths::writableLocation(QStandardPaths::HomeLocation).toStdString() + "/.prescy/stock_data"},
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

    auto addStockButton = new QPushButton{"Add Stock", this};
    // addStockButton->setIcon(QIcon(":/resources/icon/add.png"));
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
                    auto query = prescy::StockQuery{tickerData[0].toStdString(), intervalEdit->currentText().toStdString()};
                    addStock(query);
                    _registry.addStockQuery(query);
                } catch (prescy::PrescyException& e) {
                    QMessageBox::warning(this, "Error Retrieving Data", e.what());
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
    auto removeStock = new QPushButton{"Remove Stock", this};
    // removeStock->setIcon(QIcon(":/resources/icon/remove.png"));
    connect(removeStock, &QPushButton::clicked, this, [this]() {
        if (_stocks.currentItem()) {
            try {
                auto entry = qobject_cast<prescybase::StockListEntry*>(_stocks.itemWidget(_stocks.currentItem()));
                auto query = prescy::StockQuery(entry->symbol(), entry->range());
                _dataSource.removeQuery(query);
                _registry.removeStockQuery(query);
            } catch (prescy::PrescyException& e) {
                qDebug(e.what());
            }

            delete _stocks.takeItem(_stocks.currentRow());
        }
    });

    connect(&_timer, &QTimer::timeout, this, [this]() {
        if (_lastRefreshed == 5) {
            _lastRefreshedLabel.setText("Querying...");
            _lastRefreshed++;
            try {
                _dataSource.performQueries();
            } catch (prescy::PrescyException& e) {
                qDebug(e.what());
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
    layout->addWidget(addStockButton, 0, 0, 1, 1);
    layout->addWidget(removeStock, 0, 1, 1, 1);
    layout->addWidget(&_lastRefreshedLabel, 0, 3, 1, 1);
    layout->addWidget(&_stocks, 1, 0, 1, 4);

    layout->setColumnStretch(0, 0);
    layout->setColumnStretch(1, 0);
    layout->setColumnStretch(2, 1);
    layout->setColumnStretch(3, 0);

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
        qDebug(e.what());
        _timer.stop();
    }
}

void prescybase::StockWidget::addStock(const prescy::StockQuery& query)
{
    _dataSource.addQuery(query);
    _dataSource.performQueries();

    auto stockListEntry = new StockListEntry(query.symbol,
                                             _companyNames[query.symbol],
                                             query.range,
                                             this);
    auto item = new QListWidgetItem(&_stocks);
    _stocks.insertItem(_stocks.count(), item);
    _stocks.setItemWidget(item, stockListEntry);
    item->setSizeHint(stockListEntry->sizeHint());
    refreshStocks();
}
