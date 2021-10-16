#pragma once

#include <Engine/stock_indicator.hpp>

#include <QTabWidget>
#include <QWidget>

#include <unordered_map>
#include <vector>

namespace prescybase {

class IndicatorsWidget : public QWidget
{
Q_OBJECT
public:
    explicit IndicatorsWidget(const std::vector<prescyengine::StockIndicator>& indicators, QWidget *parent = nullptr);
    std::vector<prescyengine::StockIndicator> indicators();

signals:
    void indicatorAddedOrUpdated(const prescyengine::StockIndicator& indicator);
    void indicatorRemoved(const std::string& name);

private:
    void addPage(const prescyengine::StockIndicator& indicator, int index);

    QTabWidget _tabWidget;
    std::unordered_map<QWidget*, prescyengine::StockIndicator> _indicators;
};


}
