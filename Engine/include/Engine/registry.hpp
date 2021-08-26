#pragma once

#include <Engine/prescy_engine_platform.hpp>
#include <Engine/data_source/stock_query.hpp>
#include <Engine/stock_indicator.hpp>

#include <memory>
#include <string>
#include <vector>

namespace prescy {

class Prescy_Engine Registry {
public:
    explicit Registry(const std::string& path);
    Registry(const Registry&) = delete;
    Registry& operator=(const Registry&) = delete;
    ~Registry();

    void addStockQuery(const StockQuery& query);
    void removeStockQuery(const StockQuery& query);
    std::vector<StockQuery> stockQueries();

    void addStockIndicator(const StockIndicator& indicator);
    void removeStockIndicator(const StockIndicator& indicator);
    std::vector<StockIndicator> stockIndicators();

private:
    class impl;
    std::unique_ptr<impl> _impl;
};

}
