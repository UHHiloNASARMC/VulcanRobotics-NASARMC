#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGamepad>
#include "spocktcpsocket.h"
#include "inputdev/DeviceFinder.hpp"
#include "inputdev/DualshockPad.hpp"

namespace Ui {
class MainWindow;
}
class SpockDriveTrackpad;

class DeviceFinder : public QObject, public boo::DeviceFinder, public boo::IDualshockPadCallback
{
    Q_OBJECT
    std::shared_ptr<boo::DeviceBase> m_device = nullptr;
public:
    DeviceFinder()
    : boo::DeviceFinder({typeid(boo::DualshockPad)})
    {
        startScanning();
    }

    ~DeviceFinder()
    {
        if (m_device)
            m_device->closeDevice();
    }

    void deviceConnected(boo::DeviceToken& tok)
    {
        if (!m_device)
        {
            gamepadConnected();
            m_device = tok.openAndGetDevice();
            static_cast<boo::DualshockPad*>(m_device.get())->setCallback(this);
        }
    }

    void deviceDisconnected(boo::DeviceToken&, boo::DeviceBase* dev)
    {
        if (dev == m_device.get())
        {
            gamepadDisconnected();
            m_device.reset();
        }
    }

    void controllerUpdate(const boo::DualshockPadState& state)
    {
        axisLeftXChanged(state.m_leftStick[0] / 127.f - 1.f);
        axisLeftYChanged(state.m_leftStick[1] / 127.f - 1.f);
        axisRightXChanged(state.m_rightStick[0] / 127.f - 1.f);
        axisRightYChanged(state.m_rightStick[1] / 127.f - 1.f);
    }

signals:
    void gamepadConnected();
    void gamepadDisconnected();
    void axisLeftXChanged(double val);
    void axisLeftYChanged(double val);
    void axisRightXChanged(double val);
    void axisRightYChanged(double val);
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void timerEvent(QTimerEvent *event);

public slots:
    void armSliderChanged(int value);
    void bucketSliderChanged(int value);
    void driveMove(SpockDriveTrackpad* sender, QMouseEvent* ev);
    void driveRelease(SpockDriveTrackpad* sender, QMouseEvent* ev);
    void gamepadConnected();
    void gamepadDisconnected();
    void axisLeftXChanged(double value);
    void axisLeftYChanged(double value);
    void axisRightXChanged(double value);
    void axisRightYChanged(double value);
    void connectionEstablished();
    void connectionLost();
    void sendCommandPacket();

private:
    std::unique_ptr<Ui::MainWindow> ui;
    SpockTCPSocket m_socket;
    SpockCommandData m_commandData;
    DeviceFinder m_devFinder;
};

#endif // MAINWINDOW_H
