QT       += core gui widgets

CONFIG += c++17

INCLUDEPATH += \
    $$PWD/../Engine/include

DEPENDPATH += \
    $$PWD/../Engine/include

SOURCES += \
    main.cpp \
    main_window.cpp \
    stock_chart.cpp \
    stock_list_entry.cpp \
    stock_widget.cpp

HEADERS += \
    main_window.hpp \
    stock_chart.hpp \
    stock_list_entry.hpp \
    stock_widget.hpp

RESOURCES += \
    resources.qrc

macx {
LIBS += \
    -L$$OUT_PWD/../Engine/ -lEngine
}

win32 {

#RELEASE / DEBUG
CONFIG(debug, debug|release) {
    DEST_DIR = debug
} else {
    DEST_DIR = release
}

LIBS += \
    -L$$OUT_PWD/../Engine/$$DEST_DIR/ -lEngine

ENGINE.commands = $$quote(XCOPY "$$shell_path($$OUT_PWD/../Engine)" "$$shell_path($$OUT_PWD)" /c /s /d /y)

QMAKE_EXTRA_TARGETS += ENGINE

POST_TARGETDEPS += ENGINE
}

