#pragma once

#include "stock_chart.hpp"

#include <Engine/data_source/stock_data.hpp>
#include <Engine/stock_indicator.hpp>

#include <QObject>
#include <QWidget>

#include <string>
#include <unordered_map>
#include <vector>

class QLabel;

namespace prescybase {

class StockListEntry : public QWidget {
    Q_OBJECT
public:
    StockListEntry(const std::string& symbol,
                   const std::string& name,
                   const std::string& range,
                   const std::vector<prescy::StockIndicator> indicators = {},
                   QWidget* parent = 0);
    void setData(const std::vector<prescy::StockData>& data);
    void addIndicator(const prescy::StockIndicator& indicator);
    void removeIndicator(const prescy::StockIndicator& indicator);

    std::string symbol();
    std::string range();

private:
    std::string _symbol;
    std::string _range;
    std::unordered_map<prescy::StockIndicator, QLabel*, prescy::StockIndicatorHashFunction> _indicators;

    StockChart _chart;
};
}
