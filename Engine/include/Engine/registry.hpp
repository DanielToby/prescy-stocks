#pragma once

#include <Engine/data_source/stock_query.hpp>
#include <Engine/prescy_engine_platform.hpp>
#include <Engine/stock_indicator.hpp>

#include <memory>
#include <string>
#include <vector>

namespace prescyengine {

class Prescy_Engine Registry {
public:
    explicit Registry(const std::string& path);
    Registry(const Registry&) = delete;
    Registry& operator=(const Registry&) = delete;
    ~Registry();

    void addStockQuery(const StockQuery& query);
    void removeStockQuery(const StockQuery& query);
    std::vector<StockQuery> stockQueries();

    void addIndicator(const StockIndicator& indicator);
    void updateIndicator(const StockIndicator& indicator);
    void removeIndicator(const std::string& name);
    std::vector<StockIndicator> indicators();

private:
    class impl;
    std::unique_ptr<impl> _impl;
};

}
