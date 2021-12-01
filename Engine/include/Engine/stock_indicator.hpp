#pragma once
#include <Engine/prescy_engine_platform.hpp>

#include <string>

namespace prescyengine {

struct Prescy_Engine StockIndicator {
    StockIndicator();
    StockIndicator(const std::string& name, const std::string& expression);
    bool operator==(const StockIndicator& other) const;
    std::string name;
    std::string expression;
    double threshold;
};

struct Prescy_Engine StockIndicatorHashFunction {
    std::size_t operator()(const StockIndicator& query) const;
};

}
