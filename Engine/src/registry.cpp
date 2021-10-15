#include <Engine/exception.hpp>
#include <Engine/registry.hpp>
#include <src/log.hpp>

#include <fmt/format.h>
#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

#include <fstream>

namespace prescyengine {
class Registry::impl {
public:
    impl(const std::string& path) :
        _path{path},
        _stockQueries{},
        _stockIndicators{} {
        auto registryFile = std::ifstream{_path};
        if (registryFile.is_open()) {
            E_INFO(_path);
            auto json = std::string{};
            auto line = std::string{};
            while (std::getline(registryFile, line)) {
                json += line;
            }
            auto doc = rapidjson::Document{};
            doc.Parse(json.c_str());

            auto check = [](bool condition, const char* error) {
                if (!condition) {
                    throw PrescyException(error);
                }
            };

            check(doc.IsObject(), "Registry is not a json object.");
            check(doc.HasMember("stockQueries"), "No 'stockQueries' object in registry.");
            auto& stocks = doc["stockQueries"];
            check(stocks.IsArray(), "Invalid 'stockQueries' object");
            for (rapidjson::SizeType i = 0; i < stocks.Size(); ++i) {
                auto& stock = stocks[i];
                check(stock.HasMember("symbol"), "Invalid entry in stock registry");
                check(stock.HasMember("range"), "Invalid entry in stock registry");
                _stockQueries.emplace_back(stock["symbol"].GetString(), stock["range"].GetString());
            }
            check(doc.HasMember("stockIndicators"), "No 'stockIndicators' object in registry.");
            auto& indicators = doc["stockIndicators"];
            check(indicators.IsArray(), "Invalid 'stockIndicators' object");
            for (rapidjson::SizeType i = 0; i < indicators.Size(); ++i) {
                auto& indicator = indicators[i];
                check(indicator.HasMember("name"), "Invalid entry in stock registry");
                check(indicator.HasMember("expression"), "Invalid entry in stock registry");
                _stockIndicators.emplace_back(indicator["name"].GetString(), indicator["expression"].GetString());
            }

            E_INFO("Identified {} stock queries, {} indicators in registry.", _stockQueries.size(), _stockIndicators.size());
        } else {
            E_INFO("Registry '{}' not found.", _path);
        }
    }

    void writeRegistry() {
        E_TRACE("Writing to registry.");
        auto sb = rapidjson::StringBuffer{};
        auto writer = rapidjson::Writer<rapidjson::StringBuffer>(sb);

        writer.StartObject();
        writer.Key("stockQueries");
        writer.StartArray();
        for (const auto& query : _stockQueries) {
            writer.StartObject();

            writer.Key("symbol");
            writer.String(query.symbol.c_str());
            writer.Key("range");
            writer.String(query.range.c_str());

            writer.EndObject();
        }
        writer.EndArray();
        writer.Key("stockIndicators");
        writer.StartArray();
        for (const auto& indicator : _stockIndicators) {
            writer.StartObject();

            writer.Key("name");
            writer.String(indicator.name.c_str());
            writer.Key("expression");
            writer.String(indicator.expression.c_str());

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

    void addStockIndicator(const StockIndicator& indicator) {
        _stockIndicators.emplace_back(indicator);
        writeRegistry();
    }

    void removeStockIndicator(const std::string& name) {
        auto priorSize = _stockIndicators.size();
        _stockIndicators.erase(
            std::remove_if(_stockIndicators.begin(),
                           _stockIndicators.end(),
                           [&name](const StockIndicator& other) {
                               return name == other.name;
                           }),
            _stockIndicators.end());
        if (_stockIndicators.size() != priorSize) {
            writeRegistry();
        }
    }

    std::vector<StockIndicator> stockIndicators() {
        return _stockIndicators;
    }

    std::string _path;
    std::vector<StockQuery> _stockQueries;
    std::vector<StockIndicator> _stockIndicators;
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

void Registry::addStockIndicator(const StockIndicator& indicator) {
    API_CALL();
    _impl->addStockIndicator(indicator);
}

void Registry::removeStockIndicator(const std::string& name) {
    API_CALL();
    _impl->removeStockIndicator(name);
}

std::vector<StockIndicator> Registry::stockIndicators() {
    API_CALL();
    return _impl->stockIndicators();
}

}
