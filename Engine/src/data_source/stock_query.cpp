#include <Engine/data_source/stock_query.hpp>

#include <functional>

namespace prescy {

StockQuery::StockQuery() :
    symbol{""},
    range{""} {}

StockQuery::StockQuery(const std::string& symbol, const std::string& range) :
    symbol{symbol},
    range{range} {}

bool StockQuery::operator==(const StockQuery& other) const {
    return symbol == other.symbol && range == other.range;
}

size_t StockQueryHashFunction::operator()(const StockQuery& query) const {
    std::size_t h1 = std::hash<std::string>()(query.symbol);
    std::size_t h2 = std::hash<std::string>()(query.range);
    return h1 ^ h2;
}

}
