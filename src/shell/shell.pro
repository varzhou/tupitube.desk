# File generated by kdevelops qmake manager.
# -------------------------------------------
# Subdir relative project main directory: ./src/shell
# Target is an application:  ../../bin/tupi.bin

INSTALLS += tupidata \
            launcher \
            target

tupidata.target = data
tupidata.commands = cp -r data/* $(INSTALL_ROOT)/data
tupidata.path = /data/

launcher.target = ../../launcher/tupi
launcher.commands = cp ../../launcher/tupi $(INSTALL_ROOT)/bin
launcher.path = /bin/

target.path = /bin/

macx {
    CONFIG += console
    ICON = ../../launcher/icons/tupi.icns
    QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.7
    QMAKE_INFO_PLIST = ./Info.plist
    TARGET = ../../bin/Tupi
}

unix:!mac {
    INSTALLS += desktop \
                icons \
                tupiman \
                copyright

    desktop.target = ../../launcher/tupi.desktop
    desktop.commands = cp ../../launcher/tupi.desktop $(INSTALL_ROOT)/applications
    desktop.path = /applications/

    icons.target = ../../launcher/icons/tupi.png
    icons.commands = cp ../../launcher/icons/tupi.png $(INSTALL_ROOT)/pixmaps
    icons.path = /pixmaps/

    tupiman.target = ../components/help/help/man/tupi.1.gz
    tupiman.commands = cp ../components/help/help/man/tupi.1.gz $(INSTALL_ROOT)/man1
    tupiman.path = /man1/

    copyright.target = ../components/help/help/man/copyright
    copyright.commands = cp ../components/help/help/man/copyright $(INSTALL_ROOT)/share/doc/tupi
    copyright.path = /tupi/

    TARGET = ../../bin/tupi.bin
}

TRANSLATIONS += data/translations/tupi_es.ts \
                data/translations/tupi_ca.ts \
                data/translations/tupi_ru.ts \
                data/translations/tupi_cs.ts

HEADERS += ktmainwindow.h \
           ktstatusbar.h \
           ktnewproject.h \
           ktsplash.h \
           ktcrashhandler.h \
           ktcrashwidget.h \
           ktapplication.h \
           # configwizard.h \
           ktlocalprojectmanagerhandler.h

SOURCES += main.cpp \
           ktmainwindow.cpp \
           ktstatusbar.cpp \
           ktnewproject.cpp \
           ktsplash.cpp \
           ktcrashhandler.cpp \
           ktcrashwidget.cpp \
           ktapplication.cpp \
           # configwizard.cpp \
           ktmainwindow_gui.cpp \
           ktlocalprojectmanagerhandler.cpp

CONFIG += warn_on
TEMPLATE = app

linux-g{
    TARGETDEPS += ../../src/libtupi/libtupi.so \
  ../../src/libui/libui.a \
  ../../src/store/libtupistore.so \
  ../../src/net/libtupinet.so \
  ../../src/components/paintarea/libpaintarea.a \
  ../../src/components/pen/libpen.a \
  ../../src/components/kinas/libkinas.a \
  ../../src/components/help/libhelp.a \
  ../../src/components/import/libimport.a \
  ../../src/components/export/libexport.a \
  ../../src/components/exposure/libexposure.a \
  ../../src/components/timeline/libtimeline.a \
  ../../src/components/library/liblibrary.a \
  ../../src/components/colorpalette/libcolorpalette.a \
  ../../src/components/scenes/libscenes.a \
  ../../src/components/twitter/libtwitter.a
}

FRAMEWORK_DIR = ../framework
include($$FRAMEWORK_DIR/framework.pri)
include(shell_config.pri)

include(../../tupiglobal.pri)
