# File generated by kdevelop\'s qmake manager. 
# ------------------------------------------- 
# Subdir relative project main directory: ./src/libtupi
# Target is a library: tupi 

INSTALLS += target 

#headers.target = .
#headers.commands = cp *.h $(INSTALL_ROOT)/include/tupi
#headers.path = /include/tupi

target.path = /lib/

macx {
    CONFIG += plugin warn_on
}

HEADERS += tupgraphicalgorithm.h \
           tupglobal.h \
           tuppaintareaproperties.h \
           tupgradientcreator.h \
           tupgradientselector.h \
           tupgradientviewer.h \
           tuppackagehandler.h \
           tupprojectactionbar.h \
           tupbrushmanager.h \
           tupinputdeviceinformation.h \
           tupsvg2qt.h \
           tupitempreview.h \
           tupcompress.h  \
           tupsocketbase.h \
           tupxmlparserbase.h \
           tupproxyitem.h \
           tupinthash.h \
           tuppenthicknesswidget.h

SOURCES += tupgraphicalgorithm.cpp \
           tuppaintareaproperties.cpp \
           tupgradientcreator.cpp \
           tupgradientselector.cpp \
           tupgradientviewer.cpp \
           tuppackagehandler.cpp \
           tupprojectactionbar.cpp \
           tupbrushmanager.cpp \
           tupinputdeviceinformation.cpp \
           tupsvg2qt.cpp \
           tupitempreview.cpp \
           tupcompress.cpp  \
           tupsocketbase.cpp \
           tupxmlparserbase.cpp \
           tupproxyitem.cpp \
           tuppenthicknesswidget.cpp

*:!macx{
    CONFIG += dll warn_on
}

TEMPLATE = lib
TARGET = tupi

FRAMEWORK_DIR = "../framework"
include($$FRAMEWORK_DIR/framework.pri)

# QUAZIP_DIR = "../../3rdparty/quazip/"
# include($$QUAZIP_DIR/quazip.pri)

include(../../tupiglobal.pri)
