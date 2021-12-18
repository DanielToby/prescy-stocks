#pragma once

#include <Engine/stock_indicator.hpp>

#include <QPushButton>
#include <QWidget>

#include <string>

namespace prescybase {

class IndicatorThumbnail : public QWidget {
    Q_OBJECT
public:
    explicit IndicatorThumbnail(const std::string& name, const std::string& expression, double threshold, QWidget* parent = nullptr);

    void setValue(double value);
    std::string expression();

private:
    QPushButton _button;
    std::string _name;
    std::string _expression;
    double _threshold;
};

}
