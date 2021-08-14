#pragma once
#include <Engine/prescy_engine_platform.hpp>

#include <string>

namespace prescy {

struct Prescy_Engine StockData {
    StockData();
    std::string date;
    double open;
    double high;
    double low;
    double close;
    double adjacentClose;
    double volume;

};

}
