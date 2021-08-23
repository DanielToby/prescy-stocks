#include "stock_list_entry.hpp"

#include <QHBoxLayout>
#include <QLabel>
#include <QString>
#include <QVBoxLayout>

prescybase::StockListEntry::StockListEntry(const std::string& symbol, const std::string& name, const std::string& range, QWidget* parent) :
    QWidget{parent},
    _symbol{symbol},
    _range{range},
    _chart{this},
    _changedLabel{"--", this} {
    auto rangeLabel = new QLabel{QString::fromStdString(range), this};
    rangeLabel->setMaximumWidth(50);
    rangeLabel->setAlignment(Qt::AlignCenter);
    _changedLabel.setMaximumWidth(100);
    _changedLabel.setAlignment(Qt::AlignCenter);

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
    layout->addWidget(&_changedLabel);

    setLayout(layout);
}

void prescybase::StockListEntry::setData(const std::vector<prescy::StockData>& data) {
    _chart.setData(data);
    if (data.size() > 1) {
        auto percentChanged = (data[data.size() - 1].close - data[0].open) / data[0].open * 100;
        if (percentChanged < 0) {
            _changedLabel.setStyleSheet("color: red");
        } else {
            _changedLabel.setStyleSheet("color: lightGreen");
        }
        _changedLabel.setText(percentChanged > 0 ? "+" + QString::number(percentChanged, 'g', 5) + "%" : QString::number(percentChanged) + "%");
    }
}

std::string prescybase::StockListEntry::symbol()
{
    return _symbol;
}

std::string prescybase::StockListEntry::range()
{
    return _range;
}
