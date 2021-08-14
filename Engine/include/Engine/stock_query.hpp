#pragma once
#include <Engine/prescy_engine_platform.hpp>

#include <string>

namespace prescy {

struct Prescy_Engine StockQuery {
    StockQuery();
    StockQuery(const std::string& symbol, const std::string& range);

    std::string symbol;
    std::string range;
};

}
