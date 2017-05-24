message("Please execute scripts/import.sh for build dependencies")

ios|android|!isEmpty(APP_MOBILE) {
    DEFINES += APP_MOBILE
} else {
    DEFINES += APP_DESKTOP
}

TARGET = ocs-manager

TEMPLATE = app

CONFIG += \
    c++11 \
    console

CONFIG -= app_bundle

DEFINES += QT_DEPRECATED_WARNINGS

DISTFILES += $${PWD}/README.md

include($${PWD}/lib/lib.pri)
include($${PWD}/app/app.pri)
include($${PWD}/scripts/scripts.pri)
