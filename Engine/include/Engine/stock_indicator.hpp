#pragma once
#include <Engine/prescy_engine_platform.hpp>

#include <string>

namespace prescy {

struct Prescy_Engine StockIndicator {
    StockIndicator();
    StockIndicator(const std::string& name, const std::string& expression);
    bool operator==(const StockIndicator& other) const;
    std::string name;
    std::string expression;
};

struct Prescy_Engine StockIndicatorHashFunction {
    std::size_t operator()(const StockIndicator& query) const;
};

}
