#pragma once

#include <Engine/data_source/stock_data.hpp>

#include <string>
#include <vector>

namespace prescy {

double evaluateExpression(const std::vector<StockData>& data, const std::string& expression);

}
