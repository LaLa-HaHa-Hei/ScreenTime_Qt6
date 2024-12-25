QT       += core gui sql charts

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    aboutdialog.cpp \
    appsettings.cpp \
    apptrayicon.cpp \
    databasemanager.cpp \
    exeitemwidget.cpp \
    historydialog.cpp \
    main.cpp \
    mainwindow.cpp

HEADERS += \
    aboutdialog.h \
    appsettings.h \
    apptrayicon.h \
    databasemanager.h \
    exeitemwidget.h \
    historydialog.h \
    mainwindow.h

FORMS += \
    aboutdialog.ui \
    exeitemwidget.ui \
    historydialog.ui \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    ScreenTime.qrc

RC_ICONS = icons\32.ico
