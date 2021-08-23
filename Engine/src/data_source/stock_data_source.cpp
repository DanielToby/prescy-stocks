#include <Engine/data_source/stock_data_source.hpp>
#include <Engine/exception.hpp>
#include <src/log.hpp>

#include <curl/curl.h>
#include <fmt/format.h>
#include <rapidjson/document.h>

#include <atomic>
#include <memory>
#include <thread>
#include <unordered_map>

namespace {
std::size_t callBack(
    const char* in,
    std::size_t size,
    std::size_t num,
    std::string* out) {
    const std::size_t totalBytes(size * num);
    out->append(in, totalBytes);
    return totalBytes;
}
}

namespace prescy {
class StockDataSource::impl {
public:
    impl(const std::string& path) :
        _path{path},
        _data{},
        _intervals{{"1d", "1m"},
                   {"1w", "1h"},
                   {"1mo", "1d"},
                   {"3mo", "1d"},
                   {"1yr", "1d"},
                   {"5yr", "1d"}} {
    }

    void addQuery(const StockQuery& query) {
        if (_intervals.find(query.range) == _intervals.end()) {
            throw PrescyException(fmt::format("Unknown range: '{}'.", query.range));
        }
        _data[query] = {};
    }

    void removeQuery(const StockQuery& query) {
        _data.erase(query);
    }

    std::vector<StockData> parseQueryResult(const std::string& json) {
        auto data = std::vector<StockData>{};
        auto doc = rapidjson::Document{};
        doc.Parse(json.c_str());
        auto check = [](bool condition, const char* error) {
            if (!condition) {
                throw PrescyException(error);
            }
        };

        check(doc.IsObject(), "Result is not a json object.");
        check(doc.HasMember("chart"), "No 'chart' object in json object.");
        check(doc["chart"].HasMember("result"), "No 'result' object in 'chart'.");
        check(doc["chart"]["result"].IsArray(), "'result' object is not an array.");
        check(doc["chart"]["result"].Size() > 0, "'result' object is empty.");
        auto& result = doc["chart"]["result"][0];

        check(result.HasMember("timestamp"), "No 'timestamp' object in 'result'.");
        auto& timeStamps = result["timestamp"];
        check(timeStamps.IsArray(), "Invalid 'timestamps' object");
        for (rapidjson::SizeType i = 0; i < timeStamps.Size(); ++i) {
            check(timeStamps[i].IsInt(), "'timestamp' is not an integer.");
            auto dataPoint = StockData{};
            dataPoint.timeStamp = timeStamps[i].GetInt();
            data.emplace_back(dataPoint);
        }

        check(result.HasMember("indicators"), "No 'indicators' object in 'result'[0].");
        check(result["indicators"].HasMember("quote"), "No 'quote' object in 'indicators'");
        check(result["indicators"]["quote"].IsArray(), "'quote' object is not an array.");
        check(result["indicators"]["quote"].Size() > 0, "'result' object is empty.");
        auto& quote = result["indicators"]["quote"][0];

        check(quote.HasMember("close"), "No 'close' object in 'quote'[0].");
        check(quote["close"].IsArray(), "'close' object is not an array");
        auto& closes = quote["close"];
        for (rapidjson::SizeType i = 0; i < closes.Size(); ++i) {
            data[i].close = closes[i].GetDouble();
        }

        check(quote.HasMember("open"), "No 'open' object in 'quote'[0].");
        check(quote["open"].IsArray(), "'open' object is not an array");
        auto& opens = quote["open"];
        for (rapidjson::SizeType i = 0; i < opens.Size(); ++i) {
            data[i].open = opens[i].GetDouble();
        }

        check(quote.HasMember("volume"), "No 'volume' object in 'quote'[0].");
        check(quote["volume"].IsArray(), "'volume' object is not an array");
        auto& volumes = quote["volume"];
        for (rapidjson::SizeType i = 0; i < volumes.Size(); ++i) {
            data[i].volume = volumes[i].GetDouble();
        }

        check(quote.HasMember("low"), "No 'low' object in 'quote'[0].");
        check(quote["low"].IsArray(), "'low' object is not an array");
        auto& lows = quote["low"];
        for (rapidjson::SizeType i = 0; i < lows.Size(); ++i) {
            data[i].low = lows[i].GetDouble();
        }

        check(quote.HasMember("high"), "No 'high' object in 'quote'[0].");
        check(quote["high"].IsArray(), "'high' object is not an array");
        auto& highs = quote["high"];
        for (rapidjson::SizeType i = 0; i < highs.Size(); ++i) {
            data[i].high = highs[i].GetDouble();
        }

        E_INFO("Parsed {} data points from json result.", data.size());
        return data;
    }

    void performQueries() {
        if (_data.size() > 0) {
            auto numThreads = 8;
            auto numQueries = static_cast<int>(_data.size());
            std::vector<std::vector<StockQuery>> threadQueries(numThreads);
            auto i = 0;
            for (const auto& query : _data) {
                threadQueries[i % numThreads].emplace_back(query.first);
                ++i;
            }

            std::atomic<int> queriesPerformed{0};
            auto workFn = [this, &threadQueries, &queriesPerformed](int threadNum) {
                for (const auto& query : threadQueries[threadNum]) {
                    auto curl = curl_easy_init();
                    if (!curl) {
                        throw PrescyException(fmt::format("Failed to initialize curl in thread {}.", threadNum));
                    }
                    auto response = 0;
                    auto data = std::make_unique<std::string>();
                    auto url = fmt::format(
                        "https://query1.finance.yahoo.com/v7/finance/chart/{0}?"
                        "&range={1}"
                        "&interval={2}",
                        query.symbol,
                        query.range,
                        _intervals[query.range]);

                    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
                    curl_easy_setopt(curl, CURLOPT_IPRESOLVE, CURL_IPRESOLVE_V4);
                    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10);
                    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
                    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, callBack);
                    curl_easy_setopt(curl, CURLOPT_WRITEDATA, data.get());
                    curl_easy_perform(curl);
                    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response);
                    curl_easy_cleanup(curl);

                    if (response != 200) {
                        throw PrescyException(fmt::format("Curl query returned {}.", response));
                    }
                    _data[query] = parseQueryResult(*data.get());
                    ++queriesPerformed;

                    E_INFO("Performed query ({} - {}) in thread {}.", query.symbol, query.range, threadNum);
                }
            };

            std::vector<std::thread> threads;
            for (int i = 0; i < numThreads; ++i) {
                threads.emplace_back(workFn, i);
            }
            E_INFO("Started performing {} queries across {} threads.", _data.size(), numThreads);

            while (queriesPerformed < numQueries) {
                ;
            }

            for (auto& thread : threads) {
                thread.join();
            }
            E_INFO("Finished performing queries.");
        }
    }

    std::vector<StockData> data(const StockQuery& query) {
        if (_data.find(query) == _data.end()) {
            E_WARN("Data not found for query ({} - {}).", query.symbol, query.range);
            return {};
        }
        return _data[query];
    }

    std::string _path;
    std::unordered_map<StockQuery, std::vector<StockData>, StockQueryHashFunction> _data;
    std::unordered_map<std::string, std::string> _intervals;
};

StockDataSource::StockDataSource(const std::string& path) :
    _impl{new impl{path}} {
}

StockDataSource::~StockDataSource() = default;

void StockDataSource::addQuery(const StockQuery& query) {
    API_CALL();
    _impl->addQuery(query);
}

void StockDataSource::removeQuery(const StockQuery& query) {
    API_CALL();
    _impl->removeQuery(query);
}

void StockDataSource::performQueries() {
    API_CALL();
    _impl->performQueries();
}

std::vector<StockData> StockDataSource::data(const StockQuery& query) {
    API_CALL();
    return _impl->data(query);
}

}
