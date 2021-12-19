#pragma once

#include "stock_list_entry.hpp"

#include <Engine/data_source/stock_data_source.hpp>
#include <Engine/registry.hpp>
#include <Engine/stock_indicator.hpp>

#include <QTableWidget>
#include <QWidget>

#include <string>
#include <unordered_map>

namespace prescybase {

class MainWidget : public QWidget {
Q_OBJECT
public:
    MainWidget(QWidget* parent = 0);

private:
    void rebuildTable();
    void refreshTable();

    QTableWidget _stocksTable;
    int _elapsedTime;
    std::unordered_map<std::string, std::string> _companyNames;
    prescyengine::Registry _registry;
    prescyengine::StockDataSource _dataSource;
};
}
