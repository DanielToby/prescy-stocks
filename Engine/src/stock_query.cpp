#include <Engine/stock_query.hpp>

namespace prescy {

StockQuery::StockQuery() :
    symbol{""},
    range{""} {}

StockQuery::StockQuery(const std::string& symbol, const std::string& range) :
    symbol{symbol},
    range{range} {}
}
