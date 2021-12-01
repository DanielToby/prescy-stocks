#pragma once

#include "stock_chart.hpp"

#include <Engine/data_source/stock_data.hpp>

#include <QObject>
#include <QWidget>

#include <string>
class QLabel;
namespace prescybase {

class IndicatorThumbnail;

class StockListEntry : public QWidget {
    Q_OBJECT
public:
    StockListEntry(const std::string& symbol,
                   const std::string& name,
                   const std::string& range,
                   QWidget* parent = 0);
    void setData(const std::vector<prescyengine::StockData>& data);

    std::string symbol();
    std::string range();

private:
    std::string _symbol;
    std::string _range;
    StockChart _chart;
};
}
