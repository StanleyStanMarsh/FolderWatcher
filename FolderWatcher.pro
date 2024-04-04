QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

QMAKE_TARGET_COMPANY = DIPMaxMax
QMAKE_TARGET_PRODUCT = FolderWatcher
QMAKE_TARGET_COPYRIGHT = DIPMaxMax Proj.
VERSION = 0.7

CONFIG += c++20

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    'Main Window/mainwindow.cpp' \
    'Calculations/Hash Sum/HashSum.cpp' \
    'Loading Window/LoadingWindow.cpp' \
    'Main Window/ShortcutsEventFilter.cpp' \
    main.cpp

HEADERS += \
    'Main Window/mainwindow.h' \
    'Calculations/Hash Sum/HashSum.h' \
    'Loading Window/LoadingWindow.h' \
    'Main Window/ShortcutsEventFilter.h'

FORMS += \
    'Main Window/mainwindow.ui'

CONFIG += lrelease
CONFIG += embed_translations
LIBS += -lstdc++fs

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    .gitignore
