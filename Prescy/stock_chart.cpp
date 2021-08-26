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
        auto linePath = QPainterPath{};
        auto wicksPath = QPainterPath{};
        auto positiveBoxesPath = QPainterPath{};
        auto negativeBoxesPath = QPainterPath{};
        auto min = _data[0].close;
        auto max = min;
        for (const auto& point : _data) {
            min = std::min(min, point.close);
            max = std::max(max, point.close);
        }

        auto width = event->rect().width();
        auto height = event->rect().height();
        for (std::size_t i = 0; i < _data.size(); ++i) {
            if (_data[i].close != -1 && _data[i].open != -1 && _data[i].low != -1 && _data[i].high != -1) {
                auto x = (static_cast<double>(i) / static_cast<double>(_data.size())) * width;
                auto yClose = height - (height * (_data[i].close - min) / (max - min));
                auto yOpen = height - (height * (_data[i].open - min) / (max - min));
                auto yLow = height - (height * (_data[i].low - min) / (max - min));
                auto yHigh = height - (height * (_data[i].high - min) / (max - min));

                // Draws trendline
                if (i == 0) {
                    linePath.moveTo(x, yClose);
                } else {
                    linePath.lineTo(x, yClose);
                }

                // Draws wicks (bottom to top)
                wicksPath.moveTo(x, yLow);
                wicksPath.lineTo(x, yClose);
                wicksPath.moveTo(x, yOpen);
                wicksPath.lineTo(x, yHigh);

                // Draws boxes
                if (_data[i].close < _data[i].open) {
                    positiveBoxesPath.moveTo(x, yClose);
                    positiveBoxesPath.lineTo(x, yOpen);
                } else {
                    negativeBoxesPath.moveTo(x, yClose);
                    negativeBoxesPath.lineTo(x, yOpen);
                }

            }
        }

        QPainter linePainter{this};
        linePainter.setRenderHint(QPainter::Antialiasing, true);
        linePainter.setPen(QPen(isNegative ? Qt::red : Qt::green, 0.5, Qt::SolidLine, Qt::RoundCap));
        linePainter.drawPath(linePath);

        QPainter wicksPainter{this};
        wicksPainter.setPen(QPen(Qt::white, 1, Qt::SolidLine));
        wicksPainter.drawPath(wicksPath);

        QPainter negativeBoxesPainter{this};
        negativeBoxesPainter.setRenderHint(QPainter::Antialiasing, true);
        negativeBoxesPainter.setPen(QPen(Qt::red, 3, Qt::SolidLine));
        negativeBoxesPainter.drawPath(negativeBoxesPath);

        QPainter positiveBoxesPainter{this};
        positiveBoxesPainter.setRenderHint(QPainter::Antialiasing, true);
        positiveBoxesPainter.setPen(QPen(Qt::green, 3, Qt::SolidLine));
        positiveBoxesPainter.drawPath(positiveBoxesPath);
    }
}

}
