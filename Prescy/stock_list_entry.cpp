#include "stock_list_entry.hpp"
#include "indicator_thumbnail.hpp"

#include <Engine/exception.hpp>
#include <fmt/format.h>

#include <QHBoxLayout>
#include <QLabel>
#include <QString>
#include <QVBoxLayout>

namespace prescybase {

StockListEntry::StockListEntry(const std::string& symbol,
                               const std::string& name,
                               const std::string& range,
                               QWidget* parent) :
    QWidget{parent},
    _symbol{symbol},
    _range{range},
    _chart{this} {
    auto rangeLabel = new QLabel{QString::fromStdString(range), this};
    rangeLabel->setMaximumWidth(50);
    rangeLabel->setAlignment(Qt::AlignCenter);

    auto symbolLabel = new QLabel(QString::fromStdString(symbol), this);
    auto nameLabel = new QLabel(QString::fromStdString(name), this);
    auto symbolAndName = new QWidget(this);
    symbolAndName->setMaximumWidth(nameLabel->width());
    auto symbolAndNameLayout = new QVBoxLayout(this);
    symbolAndNameLayout->addWidget(symbolLabel);
    symbolAndNameLayout->addWidget(nameLabel);
    symbolAndName->setLayout(symbolAndNameLayout);

    auto layout = new QHBoxLayout(this);
    layout->addWidget(symbolAndName);
    layout->addWidget(rangeLabel);
    layout->addWidget(&_chart);

    setLayout(layout);
    setMinimumHeight(80);
}

void StockListEntry::setData(const std::vector<prescyengine::StockData>& data) {
    _chart.setData(data);
}

std::string StockListEntry::symbol() {
    return _symbol;
}

std::string StockListEntry::range() {
    return _range;
}
}
