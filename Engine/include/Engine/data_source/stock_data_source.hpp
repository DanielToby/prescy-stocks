#pragma once

#include <Engine/data_source/stock_data.hpp>
#include <Engine/data_source/stock_query.hpp>
#include <Engine/prescy_engine_platform.hpp>

#include <memory>
#include <string>
#include <vector>

namespace prescyengine {

class Prescy_Engine StockDataSource {
public:
    explicit StockDataSource();
    StockDataSource(const StockDataSource&) = delete;
    StockDataSource& operator=(const StockDataSource&) = delete;
    ~StockDataSource();

    void addQuery(const StockQuery& query);
    void removeQuery(const StockQuery& query);
    void performQueries();
    std::vector<StockData> data(const StockQuery& query);

private:
    class impl;
    std::unique_ptr<impl> _impl;
};

}
