#-------------------------------------------------
#
# Project created by QtCreator 2017-04-22T14:48:26
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = spock_gui
TEMPLATE = app
CONFIG += c++14
macx {
LIBS += -framework VLCQtCore -framework VLCQtWidgets -framework CoreFoundation
}
win32 {
LIBS += VLCQtCore.lib VLCQtWidgets.lib Winusb.lib Hid.lib Setupapi.lib User32.lib Xinput.lib
}
linux {
LIBS += -lVLCQtCore -lVLCQtWidgets -ludev
}

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += main.cpp\
        mainwindow.cpp \
    spockmotorstatusbox.cpp \
    spockdrivetrackpad.cpp \
    spockmotorslider.cpp \
    spocktcpsocket.cpp \
    inputdev/DeviceBase.cpp \
    inputdev/DeviceFinder.cpp \
    inputdev/DeviceSignature.cpp \
    inputdev/DualshockPad.cpp \
    inputdev/GenericPad.cpp \
    inputdev/InputDeviceClasses.cpp

HEADERS  += mainwindow.h \
    spockmotorstatusbox.h \
    spockcommon.h \
    spockdrivetrackpad.h \
    spockmotorslider.h \
    spocktcpsocket.h \
    inputdev/DeviceBase.hpp \
    inputdev/DeviceFinder.hpp \
    inputdev/DeviceSignature.hpp \
    inputdev/DeviceToken.hpp \
    inputdev/DualshockPad.hpp \
    inputdev/IHIDDevice.hpp \
    inputdev/IHIDListener.hpp \
    inputdev/GenericPad.hpp \
    inputdev/XInputPad.hpp

macx {
SOURCES += inputdev/HIDDeviceIOKit.cpp \
    inputdev/HIDListenerIOKit.cpp
HEADERS  += inputdev/CFPointer.hpp \
    inputdev/IOKitPointer.hpp
}
win32 {
SOURCES += inputdev/HIDDeviceWinUSB.cpp \
    inputdev/HIDListenerWinUSB.cpp
}
linux {
SOURCES += inputdev/HIDDeviceUdev.cpp \
    inputdev/HIDListenerUdev.cpp
}

FORMS    += mainwindow.ui \
    spockmotorstatusbox.ui

RESOURCES += \
    resources.qrc
