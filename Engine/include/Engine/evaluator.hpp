#pragma once

#include <Engine/data_source/stock_data.hpp>
#include <Engine/prescy_engine_platform.hpp>

#include <string>
#include <vector>

namespace prescyengine {

double Prescy_Engine evaluateExpression(const std::vector<StockData>& data, const std::string& expression);

}
