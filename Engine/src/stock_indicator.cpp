#include <Engine/stock_indicator.hpp>

namespace prescyengine {

StockIndicator::StockIndicator() :
    name{""},
    expression{""} {}

StockIndicator::StockIndicator(const std::string& name, const std::string& expression) :
    name{name},
    expression{expression} {}

bool StockIndicator::operator==(const StockIndicator& other) const {
    return name == other.name && expression == other.expression;
}

size_t StockIndicatorHashFunction::operator()(const StockIndicator& indicator) const {
    std::size_t h1 = std::hash<std::string>()(indicator.name);
    std::size_t h2 = std::hash<std::string>()(indicator.expression);
    return h1 ^ h2;
}

}
