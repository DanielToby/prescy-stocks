#pragma once

#include <Engine/prescy_engine_platform.hpp>
#include <Engine/data_source/stock_data.hpp>
#include <Engine/stock_query.hpp>

#include <memory>
#include <string>
#include <vector>

namespace prescy {

class Prescy_Engine StockDataSource {
public:
    explicit StockDataSource(const std::string& path);
    StockDataSource(const StockDataSource&) = delete;
    StockDataSource& operator=(const StockDataSource&) = delete;
    ~StockDataSource();

    void addQuery(const StockQuery& query);
    void removeQuery(const StockQuery& query);
    void performQueries();
    std::vector<prescy::StockData> data(const StockQuery& query);

private:
    class impl;
    std::unique_ptr<impl> _impl;
};

}
