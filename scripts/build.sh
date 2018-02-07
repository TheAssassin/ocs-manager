#!/bin/bash

PKGNAME='ocs-manager'

BUILDTYPE=''
if [ "${1}" ]; then
    BUILDTYPE="${1}"
fi

PROJDIR="$(cd "$(dirname "${0}")/../" && pwd)"

BUILDVER="$(cd "${PROJDIR}" && git describe --always)"

BUILDDIR="${PROJDIR}/build_${PKGNAME}_${BUILDVER}_${BUILDTYPE}"

SRCARCHIVE="${BUILDDIR}/${PKGNAME}.tar.gz"

export_srcarchive() {
    filepath="${1}"
    $(cd "${PROJDIR}" && git archive --prefix="${PKGNAME}/" --output="${filepath}" HEAD)
}

build_appimage() {
    cd "${PROJDIR}"
    mkdir -p "${BUILDDIR}"
    export_srcarchive "${SRCARCHIVE}"

    tar -xzf "${SRCARCHIVE}" -C "${BUILDDIR}"
    cp "${PROJDIR}/pkg/appimage/appimage.sh" "${BUILDDIR}/${PKGNAME}"
    cd "${BUILDDIR}/${PKGNAME}"
    sh appimage.sh
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
