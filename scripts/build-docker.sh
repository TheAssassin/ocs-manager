#!/bin/bash

PKGNAME='ocs-manager'

PKGUSER='pkgbuilder'

BUILDTYPE=''
if [ "${1}" ]; then
    BUILDTYPE="${1}"
fi

PROJDIR="$(cd "$(dirname "${0}")/../" && pwd)"

BUILDSCRIPT="${PROJDIR}/scripts/build.sh"

TRANSFERLOG="${PROJDIR}/transfer.log"

transfer_file() {
    filepath="${1}"
    if [ -f "${filepath}" ]; then
        filename="$(basename "${filepath}")"
        echo "Uploading ${filename}" >> "${TRANSFERLOG}"
        curl -T "${filepath}" "https://transfer.sh/${filename}" >> "${TRANSFERLOG}"
        echo "" >> "${TRANSFERLOG}"
    fi
}

build_appimage() {
    # docker-image: ubuntu:14.04

    apt update -qq
    apt -y install software-properties-common
    apt -y install curl git
    #apt -y install build-essential qt5-default libqt5websockets5-dev
    #apt -y install cmake libssl-dev libcurl4-gnutls-dev libxpm-dev
    apt -y install libssl1.0.0 zlib1g
    apt -y install unzip p7zip

    add-apt-repository -y ppa:beineri/opt-qt593-trusty
    echo 'deb http://download.opensuse.org/repositories/home:/TheAssassin:/AppImageLibraries/xUbuntu_14.04/ /' > /etc/apt/sources.list.d/curl-httponly.list
    curl -fsSL https://download.opensuse.org/repositories/home:TheAssassin:AppImageLibraries/xUbuntu_14.04/Release.key | apt-key add -
    apt update -qq

    apt -y install build-essential libgl1-mesa-dev qt59base qt59websockets

    curl -fsSL https://cmake.org/files/v3.10/cmake-3.10.0-rc5-Linux-x86_64.tar.gz | tar -xz --strip-components=1 -C /
    apt -y install libssl-dev libcurl3 libcurl3-gnutls libcurl4-gnutls-dev libxpm-dev

    useradd -m ${PKGUSER}
    export HOME="/home/${PKGUSER}"
    chown -R ${PKGUSER}:${PKGUSER} "${PROJDIR}"

    su -c "source /opt/qt59/bin/qt59-env.sh && sh "${BUILDSCRIPT}" ${BUILDTYPE}" ${PKGUSER}

    transfer_file "$(find "${PROJDIR}/build_"*${BUILDTYPE} -type f -name "${PKGNAME}*.AppImage")"
}

build_snap() {
    echo 'Not implemented yet'
}

build_flatpak() {
    echo 'Not implemented yet'
}

if [ "${BUILDTYPE}" = 'appimage' ]; then
    build_appimage
elif [ "${BUILDTYPE}" = 'snap' ]; then
    build_snap
elif [ "${BUILDTYPE}" = 'flatpak' ]; then
    build_flatpak
else
    echo "sh $(basename "${0}") [appimage|snap|flatpak]"
    exit 1
fi
