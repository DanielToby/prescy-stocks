#include "indicator_thumbnail.hpp"

#include <QHBoxLayout>

namespace prescybase {

IndicatorThumbnail::IndicatorThumbnail(const std::string& name, double midPoint, QWidget* parent) :
    QWidget(parent),
    _button{"--", this},
    _name(name),
    _threshold(midPoint)
{
    auto palette = _button.palette();
    palette.setColor(QPalette::Button, QColor(Qt::lightGray));
    _button.setAutoFillBackground(true);
    _button.setPalette(palette);
    _button.setFlat(true);
    _button.update();
    _button.setMaximumWidth(100);
    _button.setMaximumHeight(80);

    auto layout = new QHBoxLayout(this);
    layout->addWidget(&_button);
    layout->setAlignment(&_button, Qt::AlignCenter);
    setMinimumWidth(140);
}

void IndicatorThumbnail::setName(const std::string &name) {
    _name = name;
}

void IndicatorThumbnail::setThreshold(double threshold) {
    _threshold = threshold;
}

void IndicatorThumbnail::setValue(double value)
{
    auto palette = _button.palette();
    if (value < _threshold) {
        palette.setColor(QPalette::Button, QColor(Qt::red));
        _button.setText("-" + QString::number(value) + "%");
    } else {
        palette.setColor(QPalette::Button, QColor(Qt::darkGreen));
        _button.setText("+" + QString::number(value) + "%");
    }
    _button.setAutoFillBackground(true);
    _button.setPalette(palette);
    _button.update();
}

}

