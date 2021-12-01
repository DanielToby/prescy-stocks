#include <Engine/stock_indicator.hpp>

namespace prescyengine {

StockIndicator::StockIndicator() :
    name{""},
    expression{""},
    threshold{0} {}

StockIndicator::StockIndicator(const std::string& name, const std::string& expression) :
    name{name},
    expression{expression},
    threshold{0} {}

bool StockIndicator::operator==(const StockIndicator& other) const {
    return name == other.name;
}

size_t StockIndicatorHashFunction::operator()(const StockIndicator& indicator) const {
    return std::hash<std::string>()(indicator.name);
}

}
