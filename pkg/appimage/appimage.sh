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

install -D -m 755 /lib/x86_64-linux-gnu/libssl.so.1.0.0 ${PKGNAME}.AppDir/usr/lib/libssl.so.1.0.0
install -D -m 755 /lib/x86_64-linux-gnu/libcrypto.so.1.0.0 ${PKGNAME}.AppDir/usr/lib/libcrypto.so.1.0.0
install -D -m 755 /lib/x86_64-linux-gnu/libz.so.1 ${PKGNAME}.AppDir/usr/lib/libz.so.1

./squashfs-root/AppRun ${PKGNAME}.AppDir/usr/share/applications/${PKGNAME}.desktop -appimage

mv *.AppImage ${PKGNAME}-${PKGVER}-${PKGREL}-x86_64.AppImage
