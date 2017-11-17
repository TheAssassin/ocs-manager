include($${PWD}/qtlib/qtlib.pri)

unix:!ios:!android {
    INCLUDEPATH += $${PWD}/AppImageUpdate/include

    DEPENDPATH += $${PWD}/AppImageUpdate/include

    LIBS += \
        -L$${PWD}/AppImageUpdate-prebuilt/src/ -lappimageupdate \
        -L$${PWD}/AppImageUpdate-prebuilt/lib/zsync2/src/ -lzsync2 \
        -L$${PWD}/AppImageUpdate-prebuilt/lib/ -lcpr \
        -L$${PWD}/AppImageUpdate-prebuilt/lib/zsync2/lib/zlib/ -lz \
        -L$${PWD}/AppImageUpdate-prebuilt/lib/zsync2/lib/libzsync/ -lzsync \
        -L$${PWD}/AppImageUpdate-prebuilt/lib/zsync2/lib/librcksum/ -lrcksum

    PRE_TARGETDEPS += \
        $${PWD}/AppImageUpdate-prebuilt/src/libappimageupdate.a \
        $${PWD}/AppImageUpdate-prebuilt/lib/zsync2/src/libzsync2.a \
        $${PWD}/AppImageUpdate-prebuilt/lib/libcpr.a \
        $${PWD}/AppImageUpdate-prebuilt/lib/zsync2/lib/zlib/libz.a \
        $${PWD}/AppImageUpdate-prebuilt/lib/zsync2/lib/libzsync/libzsync.a \
        $${PWD}/AppImageUpdate-prebuilt/lib/zsync2/lib/librcksum/librcksum.a

    LIBS += -lcurl
}
