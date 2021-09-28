ADS_ROOT = $$PWD/../../../site-packages/Qt-Advanced-Docking-System

INCLUDEPATH += $$ADS_ROOT/src
DEPENDPATH += $$ADS_ROOT/src
LIBS += -L$$PWD/../lib/ads/Qt$$[QT_VERSION]-$$[QMAKE_XSPEC]

CONFIG(debug, debug|release){
    win32 {
        LIBS += -lqtadvanceddockingd
    }
    else:mac {
        LIBS += -lqtadvanceddocking_debug
    }
    else {
        LIBS += -lqtadvanceddocking
    }
}
else{
    LIBS += -lqtadvanceddocking
}
