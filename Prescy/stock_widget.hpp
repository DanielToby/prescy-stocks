#pragma once

#include "stock_list_entry.hpp"

#include <Engine/data_source/stock_data_source.hpp>
#include <Engine/stock_indicator.hpp>
#include <Engine/registry.hpp>

#include <QLabel>
#include <QWidget>

#include <QListWidget>
#include <QComboBox>
#include <QTimer>

#include <string>
#include <unordered_map>

namespace prescybase {

class StockWidget : public QWidget {
public:
    StockWidget(QWidget* parent = 0);

private:
    void refreshStocks();
    void addStock(const prescy::StockQuery& query);
    void addIndicator(const prescy::StockIndicator& indicator);

    prescy::Registry _registry;
    prescy::StockDataSource _dataSource;
    QListWidget _stocks;
    QTimer _timer;
    int _lastRefreshed;
    QLabel _lastRefreshedLabel;
    std::unordered_map<std::string, std::string> _companyNames;
    QComboBox _symbolEdit;
};
}
