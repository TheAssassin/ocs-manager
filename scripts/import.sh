#!/bin/bash

PROJDIR="$(cd "$(dirname "${0}")/../" && pwd)"

if [ ! -d "${PROJDIR}/lib/qtlib" ]; then
    git clone https://github.com/akiraohgaki/qtlib.git -b release-0.2.1 --single-branch --depth=1 "${PROJDIR}/lib/qtlib"
fi

if [ ! -d "${PROJDIR}/lib/AppImageUpdate" ]; then
    git clone https://github.com/AppImage/AppImageUpdate.git -b continuous --single-branch --depth=1 "${PROJDIR}/lib/AppImageUpdate"
    git -C "${PROJDIR}/lib/AppImageUpdate" submodule update --init --recursive #--depth=1
fi

if [ ! -d "${PROJDIR}/lib/AppImageUpdate-prebuilt" ]; then
    mkdir "${PROJDIR}/lib/AppImageUpdate-prebuilt"
    cd "${PROJDIR}/lib/AppImageUpdate-prebuilt"
    cmake "${PROJDIR}/lib/AppImageUpdate" -DUSE_SYSTEM_CURL=OFF -DBUILD_CPR_TESTS=OFF -DCMAKE_BUILD_TYPE=RelWithDebInfo
    make libappimageupdate
fi
