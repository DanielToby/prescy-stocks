#include "stock_chart.hpp"

#include <QPaintEvent>
#include <QPainter>
#include <QPainterPath>

namespace prescybase {

StockChart::StockChart(QWidget* parent) :
    QWidget{parent},
    _data{} {
}

void StockChart::setData(const std::vector<prescy::StockData>& data) {
    _data = data;
}

void StockChart::paintEvent(QPaintEvent* event) {
    if (_data.size() > 2) {
        auto isNegative = _data[0].open > _data[_data.size() - 1].close;
        auto path = QPainterPath();
        auto min = _data[0].close;
        auto max = min;
        for (const auto& point : _data) {
            min = std::min(min, point.close);
            max = std::max(max, point.close);
        }

        auto width = event->rect().width();
        auto height = event->rect().height();
        for (std::size_t i = 0; i < _data.size(); ++i) {
            auto x = (static_cast<double>(i) / static_cast<double>(_data.size())) * width;
            auto y = height - (height * (_data[i].close - min) / (max - min));
            if (i == 0) {
                path.moveTo(x, y);
            } else {
                path.lineTo(x, y);
            }
        }

        auto painter = QPainter(this);
        painter.setRenderHint(QPainter::Antialiasing, true);
        painter.setPen(QPen(isNegative ? Qt::red : Qt::green, 2, Qt::SolidLine, Qt::RoundCap));
        painter.drawPath(path);
    }
}

}
