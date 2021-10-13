CONFIG -= qt gui

TEMPLATE = lib

DEFINES += \
    PRESCY_ENGINE_EXPORTS \
    CURL_STATICLIB \
    FMT_HEADER_ONLY \
    SPDLOG_FMT_EXTERNAL

CONFIG += c++17

SOURCES += \
    src/evaluator.cpp \
    src/exception.cpp \
    src/data_source/stock_data.cpp \
    src/data_source/stock_data_source.cpp \
    src/data_source/stock_query.cpp \
    src/log.cpp \
    src/prescy_engine_platform.cpp \
    src/registry.cpp \
    src/stock_indicator.cpp

HEADERS += \
    include/Engine/data_source/stock_data.hpp \
    include/Engine/data_source/stock_data_source.hpp \
    include/Engine/data_source/stock_query.hpp \
    include/Engine/evaluator.hpp \
    include/Engine/prescy_engine_platform.hpp \
    include/Engine/exception.hpp \
    include/Engine/registry.hpp \
    include/Engine/stock_indicator.hpp

HEADERS += \
    src/log.hpp

INCLUDEPATH += \
    $$PWD/include \
    $$PWD/vendor/curl-7.70.0/include \
    $$PWD/../vendor/fmt-8.0.1/include \
    $$PWD/vendor/lua-5.1.4/include \
    $$PWD/vendor/rapidjson-1.1.0/include \
    $$PWD/vendor/spdlog-1.9.0/include

DEPENDPATH += \
    $$PWD/include \
    $$PWD/vendor/curl-7.70.0/include \
    $$PWD/../vendor/fmt-8.0.1/include \
    $$PWD/vendor/lua-5.1.4/include \
    $$PWD/vendor/rapidjson-1.1.0/include \
    $$PWD/vendor/spdlog-1.9.0/include

win32 {
LIBS += \
    -L$$PWD/vendor/curl-7.70.0/lib -llibcurl_a \
    -lws2_32 \
    -lwldap32 \
    -lcrypt32 \
    -ladvapi32 \
    -lnormaliz \
    -L$$PWD/vendor/lua-5.1.4/windows/lib -llua5.1

LUA_BIN.commands = $$quote(XCOPY "$$shell_path($$PWD/vendor/lua-5.1.4/windows/bin)" "$$shell_path($$OUT_PWD)" /c /s /d /y)

QMAKE_EXTRA_TARGETS += LUA_BIN
POST_TARGETDEPS += LUA_BIN
}

macx {
LIBS += \
    -lcurl \
    -L$$PWD/vendor/lua-5.1.4/macos/lib -llua
}

