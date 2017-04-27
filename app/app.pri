QT += \
    core \
    network \
    websockets

QT -= gui

HEADERS += \
    $${PWD}/src/websockets/websocketserver.h \
    $${PWD}/src/handlers/confighandler.h \
    $${PWD}/src/handlers/systemhandler.h \
    $${PWD}/src/handlers/ocshandler.h \
    $${PWD}/src/handlers/itemhandler.h

SOURCES += \
    $${PWD}/src/main.cpp \
    $${PWD}/src/websockets/websocketserver.cpp \
    $${PWD}/src/handlers/confighandler.cpp \
    $${PWD}/src/handlers/systemhandler.cpp \
    $${PWD}/src/handlers/ocshandler.cpp \
    $${PWD}/src/handlers/itemhandler.cpp

RESOURCES += $${PWD}/configs/configs.qrc

INCLUDEPATH += $${PWD}/src

unix:!ios:!android {
    QT += dbus
}
