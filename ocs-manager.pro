ios|android|!isEmpty(APP_MOBILE) {
    DEFINES += APP_MOBILE
} else {
    DEFINES += APP_DESKTOP
}

TARGET = ocs-manager

TEMPLATE = app

CONFIG += console

CONFIG -= app_bundle

DEFINES += QT_DEPRECATED_WARNINGS

DISTFILES += $${PWD}/README.md

include($${PWD}/lib/lib.pri)
include($${PWD}/app/app.pri)
include($${PWD}/desktop/desktop.pri)
include($${PWD}/i18n/i18n.pri)
include($${PWD}/pkg/pkg.pri)
include($${PWD}/scripts/scripts.pri)
include($${PWD}/deployment.pri)
