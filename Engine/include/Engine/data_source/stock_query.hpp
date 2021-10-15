#pragma once
#include <Engine/prescy_engine_platform.hpp>

#include <string>

namespace prescyengine {

struct Prescy_Engine StockQuery {
    StockQuery();
    StockQuery(const std::string& symbol, const std::string& range);
    bool operator==(const StockQuery& other) const;

    std::string symbol;
    std::string range;
};

struct StockQueryHashFunction {
    std::size_t operator()(const StockQuery& query) const;
};

}
