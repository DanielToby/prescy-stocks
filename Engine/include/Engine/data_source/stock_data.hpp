#pragma once
#include <Engine/prescy_engine_platform.hpp>

#include <string>

namespace prescyengine {

struct Prescy_Engine StockData {
    StockData();
    int timeStamp;
    double open;
    double close;
    double high;
    double low;
    double volume;
};

}
