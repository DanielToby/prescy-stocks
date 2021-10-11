#include "stock_list_entry.hpp"

#include <QHBoxLayout>
#include <QLabel>
#include <QString>
#include <QVBoxLayout>

prescybase::StockListEntry::StockListEntry(const std::string& symbol,
                                           const std::string& name,
                                           const std::string& range,
                                           const std::vector<prescy::StockIndicator> indicators,
                                           QWidget* parent) :
    QWidget{parent},
    _symbol{symbol},
    _range{range},
    _chart{this} {
    auto rangeLabel = new QLabel{QString::fromStdString(range), this};
    rangeLabel->setMaximumWidth(50);
    rangeLabel->setAlignment(Qt::AlignCenter);

    auto symbolLabel = new QLabel(QString::fromStdString(symbol).toUpper(), this);
    auto nameLabel = new QLabel(QString::fromStdString(name), this);

    auto symbolAndName = new QWidget(this);
    symbolAndName->setMaximumWidth(200);
    auto symbolAndNameLayout = new QVBoxLayout(this);
    symbolAndNameLayout->addWidget(symbolLabel);
    symbolAndNameLayout->addWidget(nameLabel);
    symbolAndName->setLayout(symbolAndNameLayout);

    auto layout = new QHBoxLayout(this);
    layout->addWidget(symbolAndName);
    layout->addWidget(rangeLabel);
    layout->addWidget(&_chart);

    for (const auto& indicator : indicators) {
        addIndicator(indicator);
    }

    setLayout(layout);
}

void prescybase::StockListEntry::setData(const std::vector<prescy::StockData>& data) {
    _chart.setData(data);
    if (data.size() > 1) {
        for (const auto& [indicator, label] : _indicators) {
            // get result of lua expression
            // set indicator color and label
            if (indicator.expression == "percentChange") {
                auto percentChanged = (data[data.size() - 1].close - data[0].open) / data[0].open * 100;
                if (percentChanged < 0) {
                    label->setStyleSheet("color: red");
                } else {
                    label->setStyleSheet("color: lightGreen");
                }
                label->setText(percentChanged > 0 ? "+" + QString::number(percentChanged, 'g', 5) + "%" : QString::number(percentChanged) + "%");
            }
        }
    }
}

void prescybase::StockListEntry::addIndicator(const prescy::StockIndicator& indicator) {
    auto label = new QLabel("--", this);
    label->setMaximumWidth(100);
    label->setAlignment(Qt::AlignCenter);
    this->layout()->addWidget(label);
    _indicators[indicator] = label;
}

void prescybase::StockListEntry::removeIndicator(const prescy::StockIndicator& indicator) {
    auto label = _indicators[indicator];
    this->layout()->removeWidget(label);
    _indicators.erase(indicator);
}

std::string prescybase::StockListEntry::symbol() {
    return _symbol;
}

std::string prescybase::StockListEntry::range() {
    return _range;
}
