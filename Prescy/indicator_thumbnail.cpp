#include "indicator_thumbnail.hpp"

#include <QHBoxLayout>

namespace prescybase {

IndicatorThumbnail::IndicatorThumbnail(const std::string& name, const std::string& expression, double midPoint, QWidget* parent) :
    QWidget(parent),
    _button{"--", this},
    _name(name),
    _expression(expression),
    _threshold(midPoint) {
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

void IndicatorThumbnail::setValue(double value) {
    _button.setText(QString::number(value));
    auto palette = _button.palette();
    if (value < _threshold) {
        palette.setColor(QPalette::Button, QColor(Qt::red));
    } else {
        palette.setColor(QPalette::Button, QColor(Qt::darkGreen));
    }
    _button.setAutoFillBackground(true);
    _button.setPalette(palette);
    _button.update();
}

std::string IndicatorThumbnail::expression() {
    return _expression;
}

}
