#!/bin/sh

PKGNAME='ocs-manager'
PKGVER='0.5.5'
PKGREL='1'

curl -fsSL -o linuxdeployqt.AppImage https://github.com/probonopd/linuxdeployqt/releases/download/continuous/linuxdeployqt-continuous-x86_64.AppImage
chmod 755 linuxdeployqt.AppImage
./linuxdeployqt.AppImage --appimage-extract

sh scripts/prepare.sh
qmake PREFIX=/usr
make
make INSTALL_ROOT="${PKGNAME}.AppDir" install

./squashfs-root/AppRun ${PKGNAME}.AppDir/usr/share/applications/${PKGNAME}.desktop -bundle-non-qt-libs -no-translations

install -D -m 755 /lib/x86_64-linux-gnu/libssl.so.1.0.0 ${PKGNAME}.AppDir/usr/lib/libssl.so.1.0.0
install -D -m 755 /lib/x86_64-linux-gnu/libcrypto.so.1.0.0 ${PKGNAME}.AppDir/usr/lib/libcrypto.so.1.0.0
install -D -m 755 /lib/x86_64-linux-gnu/libz.so.1 ${PKGNAME}.AppDir/usr/lib/libz.so.1

install -D -m 755 /usr/bin/unzip ${PKGNAME}.AppDir/usr/bin/unzip
install -D -m 755 /usr/lib/p7zip/7zr ${PKGNAME}.AppDir/usr/bin/7z

./squashfs-root/AppRun ${PKGNAME}.AppDir/usr/share/applications/${PKGNAME}.desktop -appimage

mv *-x86_64.AppImage ${PKGNAME}-${PKGVER}-${PKGREL}-x86_64.AppImage
