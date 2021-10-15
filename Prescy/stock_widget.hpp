#pragma once

#include "stock_list_entry.hpp"

#include <Engine/data_source/stock_data_source.hpp>
#include <Engine/registry.hpp>
#include <Engine/stock_indicator.hpp>

#include <QLabel>
#include <QWidget>

#include <QComboBox>
#include <QListWidget>
#include <QTimer>

#include <string>
#include <unordered_map>

namespace prescybase {

class StockWidget : public QWidget {
public:
    StockWidget(QWidget* parent = 0);

private:
    void refreshStocks();
    void addStock(const prescyengine::StockQuery& query);
    void addIndicator(const prescyengine::StockIndicator& indicator);

    prescyengine::Registry _registry;
    prescyengine::StockDataSource _dataSource;
    QListWidget _stocks;
    QTimer _timer;
    int _lastRefreshed;
    QLabel _lastRefreshedLabel;
    std::unordered_map<std::string, std::string> _companyNames;
    QComboBox _symbolEdit;
};
}
