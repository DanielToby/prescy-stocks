#pragma once

#include <Engine/data_source/stock_data.hpp>

#include <QObject>
#include <QWidget>

namespace prescybase {

class StockChart : public QWidget {
    Q_OBJECT
public:
    explicit StockChart(QWidget* parent = 0);
    void setData(const std::vector<prescyengine::StockData>& data);

protected:
    void paintEvent(QPaintEvent* event);

private:
    std::vector<prescyengine::StockData> _data;
};

}
