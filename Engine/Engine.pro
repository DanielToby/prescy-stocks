CONFIG -= qt gui

TEMPLATE = lib

DEFINES += \
    PRESCY_ENGINE_EXPORTS \
    CURL_STATICLIB \
    FMT_HEADER_ONLY \
    SPDLOG_FMT_EXTERNAL

QMAKE_CXXFLAGS += /std:c++latest

CONFIG += c++20

SOURCES += \
    src/exception.cpp \
    src/data_source/stock_data.cpp \
    src/data_source/stock_data_source.cpp \
    src/data_source/stock_query.cpp \
    src/log.cpp \
    src/prescy_engine_platform.cpp \
    src/registry.cpp

HEADERS += \
    include/Engine/data_source/stock_data.hpp \
    include/Engine/data_source/stock_data_source.hpp \
    include/Engine/data_source/stock_query.hpp \
    include/Engine/prescy_engine_platform.hpp \
    include/Engine/exception.hpp \
    include/Engine/registry.hpp

HEADERS += \
    src/log.hpp

INCLUDEPATH += \
    $$PWD/include \
    $$PWD/vendor/curl-7.70.0/include \
    $$PWD/vendor/fmt-8.0.1/include \
    $$PWD/vendor/rapidjson-1.1.0/include \
    $$PWD/vendor/spdlog-1.9.0/include

DEPENDPATH += \
    $$PWD/include \
    $$PWD/vendor/curl-7.70.0/include \
    $$PWD/vendor/fmt-8.0.1/include \
    $$PWD/vendor/rapidjson-1.1.0/include \
    $$PWD/vendor/spdlog-1.9.0/include

LIBS += \
    -L$$PWD/vendor/curl-7.70.0/lib -llibcurl_a \
    -lws2_32 \
    -lwldap32 \
    -lcrypt32 \
    -ladvapi32 \
    -lnormaliz
