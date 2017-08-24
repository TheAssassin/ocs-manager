#!/bin/bash

sh scripts/import.sh

qmake PREFIX="/usr"
make
make INSTALL_ROOT="ocs-manager.AppDir" install

curl -L -o linuxdeployqt.AppImage "https://github.com/probonopd/linuxdeployqt/releases/download/continuous/linuxdeployqt-continuous-x86_64.AppImage"
chmod 755 linuxdeployqt.AppImage

./linuxdeployqt.AppImage ocs-manager.AppDir/usr/share/applications/ocs-manager.desktop -bundle-non-qt-libs -no-translations
./linuxdeployqt.AppImage ocs-manager.AppDir/usr/share/applications/ocs-manager.desktop -appimage
