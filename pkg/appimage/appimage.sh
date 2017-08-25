#!/bin/sh

PKGNAME='ocs-manager'
PKGVER='0.4.4'
PKGREL='1'

curl -L -o linuxdeployqt "https://github.com/probonopd/linuxdeployqt/releases/download/continuous/linuxdeployqt-continuous-x86_64.AppImage"
chmod 755 linuxdeployqt
./linuxdeployqt --appimage-extract

sh scripts/import.sh
qmake PREFIX="/usr"
make
make INSTALL_ROOT="${PKGNAME}.AppDir" install

./squashfs-root/AppRun ${PKGNAME}.AppDir/usr/share/applications/${PKGNAME}.desktop -bundle-non-qt-libs -no-translations
./squashfs-root/AppRun ${PKGNAME}.AppDir/usr/share/applications/${PKGNAME}.desktop -appimage

mv *.AppImage ${PKGNAME}-${PKGVER}-${PKGREL}-x86_64.AppImage
