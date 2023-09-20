#-------------------------------------------------
#
# Qwt Library configuration file
#
#-------------------------------------------------

# QWT_VER = 5.2.3
QWT_VER = 6.1.3
QWT_VER = 6.4.0-svn

contains(QWT_VER,^5\\..*\\..*) {
    VER_SFX     = 5
    UNIX_SFX    = -qt4
} else {
    VER_SFX     =
    UNIX_SFX    =
}

unix {
    QWT_PATH = /usr/local/qwt-6.4.0-svn
    QWT_INC_PATH = $${QWT_PATH}/include/qwt$${UNIX_SFX}
    QWT_LIB  = qwt$${UNIX_SFX}
}

win32 {
    win32-x-g++ {
        QWT_PATH = /usr/qwt$${VER_SFX}-win
    } else {
        QWT_PATH = C:/Qt/qwt-$${QWT_VER}
    }
    QWT_INC_PATH = $${QWT_PATH}/include
    CONFIG(debug,debug|release) {
        DEBUG_SFX = d
    } else {
        DEBUG_SFX =
    }
    QWT_LIB = qwt$${DEBUG_SFX}$${VER_SFX}
}

INCLUDEPATH += /usr/local/qwt-6.4.0-svn/include
#INCLUDEPATH += /usr/local/qwt-6.1.3/include
LIBS += -L$${QWT_PATH}/lib -l$${QWT_LIB}
