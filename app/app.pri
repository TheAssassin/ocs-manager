CONFIG += c++11

QT += \
    core \
    gui \
    network \
    websockets

HEADERS += \
    $${PWD}/src/websockets/websocketserver.h \
    $${PWD}/src/handlers/confighandler.h \
    $${PWD}/src/handlers/systemhandler.h \
    $${PWD}/src/handlers/ocsapihandler.h \
    $${PWD}/src/handlers/itemhandler.h

SOURCES += \
    $${PWD}/src/main.cpp \
    $${PWD}/src/websockets/websocketserver.cpp \
    $${PWD}/src/handlers/confighandler.cpp \
    $${PWD}/src/handlers/systemhandler.cpp \
    $${PWD}/src/handlers/ocsapihandler.cpp \
    $${PWD}/src/handlers/itemhandler.cpp

RESOURCES += $${PWD}/configs/configs.qrc

INCLUDEPATH += $${PWD}/src

unix:!ios:!android {
    QT += dbus

    HEADERS += \
        $${PWD}/src/handlers/desktopthemehandler.h \
        $${PWD}/src/desktopthemes/kdetheme.h \
        $${PWD}/src/desktopthemes/gnometheme.h \
        $${PWD}/src/desktopthemes/xfcetheme.h \
        $${PWD}/src/desktopthemes/matetheme.h

    SOURCES += \
        $${PWD}/src/handlers/desktopthemehandler.cpp \
        $${PWD}/src/desktopthemes/kdetheme.cpp \
        $${PWD}/src/desktopthemes/gnometheme.cpp \
        $${PWD}/src/desktopthemes/xfcetheme.cpp \
        $${PWD}/src/desktopthemes/matetheme.cpp
}
