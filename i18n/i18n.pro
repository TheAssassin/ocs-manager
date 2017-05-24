SOURCES += $$system(find $${PWD}/../app -type f -name "*.cpp" -or -name "*.qml" -or -name "*.js")

TRANSLATIONS += \
    $${PWD}/messages.ts \
    $${PWD}/messages.en_US.ts

include($${PWD}/i18n.pri)
