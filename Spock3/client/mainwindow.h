#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "spocktcpsocket.h"

namespace Ui {
class MainWindow;
}
class SpockDriveTrackpad;

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
    void connectionEstablished();
    void connectionLost();
    void sendCommandPacket();

private:
    std::unique_ptr<Ui::MainWindow> ui;
    SpockTCPSocket m_socket;
    SpockCommandData m_commandData;
};

#endif // MAINWINDOW_H
