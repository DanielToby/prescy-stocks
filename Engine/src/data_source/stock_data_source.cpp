#include <Engine/data_source/stock_data_source.hpp>
#include <Engine/exception.hpp>
#include <src/log.hpp>

#include <curl/curl.h>
#include <fmt/format.h>

#include <atomic>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <thread>
#include <unordered_map>

namespace prescy {
class StockDataSource::impl {
public:
    impl(const std::string& path) :
        _path{path},
        _queries{},
        _intervals{{"1d", "1h"},
                   {"1w", "1h"},
                   {"1mo", "1d"},
                   {"3mo", "1d"},
                   {"1yr", "1d"},
                   {"5yr", "1d"}} {
    }

    std::string fileName(const StockQuery& query) {
        return fmt::format("{0}/prescy_data__{1}_{2}.csv", _path, query.symbol, query.range);
    }

    void addQuery(const StockQuery& query) {
        if (_intervals.find(query.range) == _intervals.end()) {
            throw PrescyException(fmt::format("Unknown range: '{}'.", query.range));
        }
        _queries.emplace_back(query);
    }

    void removeQuery(const StockQuery& query) {
        _queries.erase(
            std::remove_if(_queries.begin(),
                           _queries.end(),
                           [&query](const StockQuery& other) {
                               return query.symbol == other.symbol && query.range == other.range;
                           }),
            _queries.end());
    }

    void performQueries() {
        if (_queries.size() > 0) {
            auto numThreads = 8;
            auto numQueries = static_cast<int>(_queries.size());
            std::vector<std::vector<StockQuery>> threadQueries(numThreads);
            for (std::size_t i = 0; i < _queries.size(); ++i) {
                threadQueries[i % numThreads].emplace_back(_queries[i]);
            }

            std::atomic<int> queriesPerformed{0};
            auto workFn = [this, &threadQueries, &queriesPerformed](int threadNum) {
                for (const auto& query : threadQueries[threadNum]) {
                    auto curl = curl_easy_init();
                    if (curl) {
                        auto url = fmt::format(
                            "https://query1.finance.yahoo.com/v7/finance/download/{0}?"
                            "range={1}&"
                            "interval={2}&"
                            "events=history&"
                            "includeAdjustedClose=false",
                            query.symbol,
                            query.range,
                            _intervals[query.range]);
                        auto fp = fopen(fileName(query).c_str(), "wb");
                        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
                        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, nullptr);
                        curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
                        auto result = curl_easy_perform(curl);
                        if (result != CURLE_OK) {
                            throw PrescyException(fmt::format("Curl error: {}.", result));
                        }
                        curl_easy_cleanup(curl);
                        fclose(fp);
                    } else {
                        E_WARN("Failed to initialize curl in thread {}.", threadNum);
                    }
                    ++queriesPerformed;
                    E_INFO("Performed query {} - {} in thread {}", query.symbol, query.range, threadNum);
                }
            };

            std::vector<std::thread> threads;
            for (int i = 0; i < numThreads; ++i) {
                threads.emplace_back(workFn, i);
            }
            E_INFO("Started performing {} queries across {} threads.", _queries.size(), numThreads);

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
        // Yahoo finance intervals: [1m, 2m, 5m, 15m, 30m, 60m, 90m, 1h, 1d, 5d, 1wk, 1mo, 3mo]
        auto data = std::vector<StockData>{};
        auto file = fileName(query);
        auto stream = std::ifstream{file};
        if (stream.is_open()) {
            auto line = std::string{};
            std::getline(stream, line);

            if (line != "Date,Open,High,Low,Close,Adj Close,Volume") {
                auto what = line;
                while (std::getline(stream, line)) {
                    what += line;
                }
                stream.close();
                throw PrescyException(what);
            }

            while (std::getline(stream, line)) {
                auto lineStream = std::stringstream{line};
                auto cell = std::string{};
                auto stock = StockData{};
                auto i = 0;
                while (std::getline(lineStream, cell, ',')) {
                    switch (i) {
                    case (0):
                        stock.date = cell;
                        break;
                    case (1):
                        stock.open = std::stod(cell);
                        break;
                    case (2):
                        stock.high = std::stod(cell);
                        break;
                    case (3):
                        stock.low = std::stod(cell);
                        break;
                    case (4):
                        stock.close = std::stod(cell);
                        break;
                    case (5):
                        stock.adjacentClose = std::stod(cell);
                        break;
                    case (6):
                        stock.volume = std::stod(cell);
                        break;
                    }
                    ++i;
                }
                data.emplace_back(stock);
            }
            stream.close();
        } else {
            E_WARN("Data for query {}, {} not found.", query.symbol, query.range);
        }
        E_INFO("Fetched dataset with size {} from file '{}'.", data.size(), file);

        return data;
    }

    std::string _path;
    std::vector<StockQuery> _queries;
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
