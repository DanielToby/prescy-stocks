#pragma once

#include <Engine/stock_indicator.hpp>

#include <QWidget>
#include <QPushButton>

#include <string>

namespace prescybase {

class IndicatorThumbnail : public QWidget
{
Q_OBJECT
public:
    explicit IndicatorThumbnail(const std::string& name, double threshold, QWidget *parent = nullptr);

    std::string name();
    void setName(const std::string& name);
    void setThreshold(double threshold);
    void setValue(double value);

private:
    QPushButton _button;
    std::string _name;
    double _threshold;
};


}
