#include "stock_list_entry.hpp"

#include <Engine/evaluator.hpp>
#include <Engine/exception.hpp>
#include <fmt/format.h>

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
            try {
                auto result = prescy::evaluateExpression(data, indicator.expression);
                result < 0 ? label->setStyleSheet("color: red") : label->setStyleSheet("color: lightGreen");
                label->setText(result > 0 ? "+" + QString::number(result, 'g', 5) + "%" : QString::number(result) + "%");
            } catch (prescy::PrescyException& e) {
                label->setText("--");
                qDebug("%s\n", e.what());
            }
        }
    }
}

void prescybase::StockListEntry::addIndicator(const prescy::StockIndicator& indicator) {
    auto label = new QLabel("--", this);
    auto toolTip = fmt::format("{}<br/><br/>{}", indicator.name, indicator.expression);
    label->setToolTip(QString::fromStdString(toolTip));
    label->setMaximumWidth(100);
    label->setAlignment(Qt::AlignCenter);
    this->layout()->addWidget(label);
    _indicators[indicator] = label;
}

void prescybase::StockListEntry::removeIndicator(const std::string& name) {
    for (auto iterator = _indicators.begin(); iterator != _indicators.end(); ++iterator) {
        if (iterator->first.name == name) {
            this->layout()->removeWidget(iterator->second);
            _indicators.erase(iterator);
        }
    }
}

std::string prescybase::StockListEntry::symbol() {
    return _symbol;
}

std::string prescybase::StockListEntry::range() {
    return _range;
}
