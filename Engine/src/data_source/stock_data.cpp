#include <Engine/data_source/stock_data.hpp>

namespace prescy {

StockData::StockData() :
    date{""},
    open{-1},
    high{-1},
    low{-1},
    close{-1},
    adjacentClose{-1},
    volume{-1} {}
}