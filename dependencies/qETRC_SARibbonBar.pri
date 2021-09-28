SARibbon_root = $$PWD/../../../site-packages/SARibbon/
SARibbon_bin_dir_base = $$PWD/../lib/SARibbon/Qt$$[QT_VERSION]-$$[QMAKE_XSPEC]
SARIBBON_BIN_DIR = $$SARibbon_bin_dir_base

#CONFIG(debug, debug|release){
#    contains(QT_ARCH, i386) {
#        SARIBBON_BIN_DIR = $${SARibbon_bin_dir_base}_debug
#    }else {
#        SARIBBON_BIN_DIR = $${SARibbon_bin_dir_base}_debug_64
#    }
#}else{
#    contains(QT_ARCH, i386) {
#        SARIBBON_BIN_DIR = $${SARibbon_bin_dir_base}_release
#    }else {
#        SARIBBON_BIN_DIR = $${SARibbon_bin_dir_base}_release_64
#    }
#}

#生成一个区别debug和release模式的lib名,输入一个lib名字
defineReplace(saRibbonLibNameMake) {
    LibName = $$1
    CONFIG(debug, debug|release){
        LibName = $${LibName}d
    }else{
        LibName = $${LibName}
    }
    return ($${LibName})
}

SARIBBON_LIB_NAME=$$saRibbonLibNameMake(SARibbonBar)


INCLUDEPATH += $${SARibbon_root}/src/SARibbonBar
DEPENDPATH += $${SARibbon_root}/src/SARibbonBar
LIBS += -L$${SARIBBON_BIN_DIR} -l$${SARIBBON_LIB_NAME}
