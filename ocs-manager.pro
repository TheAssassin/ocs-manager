message("Please execute scripts/import.sh for build dependencies")

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
