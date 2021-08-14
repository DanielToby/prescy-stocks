#include <Engine/exception.hpp>
#include <Engine/registry.hpp>
#include <src/log.hpp>

#include <fmt/format.h>
#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

#include <fstream>

namespace prescy {
class Registry::impl {
public:
    impl(const std::string& path) :
        _path{path} {
        auto registryFile = std::ifstream{_path};
        if (registryFile.is_open()) {
            auto json = std::string{};
            auto line = std::string{};
            while (std::getline(registryFile, line)) {
                json += line;
            }
            rapidjson::Document doc;
            doc.Parse(json.c_str());

            auto check = [](bool condition, const char* error) {
                if (!condition) {
                    throw PrescyException(error);
                }
            };

            check(doc.IsObject(), "Registry is not a json object.");
            check(doc.HasMember("stockQueries"), "No 'stocks' object in registry.");
            auto& stocks = doc["stockQueries"];
            check(stocks.IsArray(), "Invalid 'stocks' object");
            for (rapidjson::SizeType i = 0; i < stocks.Size(); ++i) {
                auto& stock = stocks[i];
                check(stock.HasMember("symbol"), "Invalid entry in stock registry");
                auto symbol = stock["symbol"].GetString();
                check(stock.HasMember("range"), "Invalid entry in stock registry");
                auto range = stock["range"].GetString();

                _stockQueries.emplace_back(symbol, range);
            }
            E_INFO("Identified {} stock queries, {} indicators in registry.", _stockQueries.size(), _indicators.size());
        } else {
            E_INFO("Registry '{}' not found.", _path);
        }
    }

    void writeRegistry() {
        E_TRACE("Writing to registry.");
        auto sb = rapidjson::StringBuffer{};
        auto writer = rapidjson::Writer(sb);

        writer.StartObject();
        writer.Key("stockQueries");
        writer.StartArray();
        for (const auto& stock : _stockQueries) {
            writer.StartObject();

            writer.Key("symbol");
            writer.String(stock.symbol.c_str());
            writer.Key("range");
            writer.String(stock.range.c_str());

            writer.EndObject();
        }
        writer.EndArray();
        writer.EndObject();

        E_TRACE(sb.GetString());

        auto registryFile = std::ofstream{_path};
        if (registryFile.is_open()) {
            registryFile << sb.GetString();
            registryFile.close();
        } else {
            throw PrescyException(fmt::format("Failed to write registry file '{}'.", _path));
        }
    }

    void addStockQuery(const StockQuery& query) {
        _stockQueries.emplace_back(query);
        writeRegistry();
    }

    void removeStockQuery(const StockQuery& query) {
        auto priorSize = _stockQueries.size();
        _stockQueries.erase(
            std::remove_if(_stockQueries.begin(),
                           _stockQueries.end(),
                           [&query](const StockQuery& other) {
                               return query.symbol == other.symbol && query.range == other.range;
                           }),
            _stockQueries.end());
        if (_stockQueries.size() != priorSize) {
            writeRegistry();
        }
    }

    std::vector<StockQuery> stockQueries() {
        return _stockQueries;
    }

    std::string _path;
    std::vector<StockQuery> _stockQueries;
    std::vector<std::string> _indicators;
};

Registry::Registry(const std::string& path) :
    _impl{new impl{path}} {
}

Registry::~Registry() = default;

void Registry::addStockQuery(const StockQuery& query) {
    API_CALL();
    _impl->addStockQuery(query);
}

void Registry::removeStockQuery(const StockQuery& query) {
    API_CALL();
    _impl->removeStockQuery(query);
}

std::vector<StockQuery> Registry::stockQueries() {
    API_CALL();
    return _impl->stockQueries();
}

}
