#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMouseEvent>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_socket("raspberrypi.local", this)
{
    ui->setupUi(this);
    ui->connectionLabel->setText("Connecting to raspberrypi.local");

    connect(&m_devFinder, SIGNAL(gamepadConnected()), this, SLOT(gamepadConnected()));
    connect(&m_devFinder, SIGNAL(gamepadDisconnected()), this, SLOT(gamepadDisconnected()));
    connect(&m_devFinder, SIGNAL(axisLeftXChanged(double)), this, SLOT(axisLeftXChanged(double)));
    connect(&m_devFinder, SIGNAL(axisLeftYChanged(double)), this, SLOT(axisLeftYChanged(double)));
    connect(&m_devFinder, SIGNAL(axisRightXChanged(double)), this, SLOT(axisRightXChanged(double)));
    connect(&m_devFinder, SIGNAL(axisRightYChanged(double)), this, SLOT(axisRightYChanged(double)));

    setFixedSize(600, 550);
    startTimer(20);
}

void MainWindow::timerEvent(QTimerEvent*)
{
    m_socket.reestablishConnection();
    sendCommandPacket();
    SpockStatusData data = m_socket.getStatusData();
    ui->flMotorStat->setStatusData(data.frontLeftMotorStatus);
    ui->frMotorStat->setStatusData(data.frontRightMotorStatus);
    ui->blMotorStat->setStatusData(data.rearLeftMotorStatus);
    ui->brMotorStat->setStatusData(data.rearRightMotorStatus);
    ui->armMotorStat->setStatusData(data.armMotorStatus);
    ui->bucketMotorStat->setStatusData(data.bucketMotorStatus);
}

void MainWindow::armSliderChanged(int value)
{
    //printf("arm %d\n", value);
    m_commandData.armAngleBias = value;
}

void MainWindow::bucketSliderChanged(int value)
{
    //printf("bucket %d\n", value);
    m_commandData.linearActuatorBias = value;
}

void MainWindow::driveMove(SpockDriveTrackpad* sender, QMouseEvent* ev)
{
    //printf("move %f %f\n", ev->x() / float(sender->width()), ev->y() / float(sender->height()));
    float x = ev->x() / float(sender->width()) * 2.f - 1.f;
    float y = std::max(0.f, std::min(ev->y() / float(sender->height()), 1.f)) * 2.f - 1.f;
    float leftMul = std::max(0.f, std::min(1.f - x, 1.f));
    float rightMul = std::max(0.f, std::min(x + 1.f, 1.f));
    m_commandData.leftThrottle = leftMul * -y * 1023.f;
    m_commandData.rightThrottle = rightMul * -y * 1023.f;
    //printf("%d %d\n", m_commandData.leftThrottle, m_commandData.rightThrottle);
}

void MainWindow::driveRelease(SpockDriveTrackpad*, QMouseEvent*)
{
    //printf("release %f %f\n", ev->x() / float(sender->width()), ev->y() / float(sender->height()));
    m_commandData.leftThrottle = 0;
    m_commandData.rightThrottle = 0;
}

void MainWindow::gamepadConnected()
{
    //printf("Connected\n");
}

void MainWindow::gamepadDisconnected()
{
    //printf("Disconnected\n");
}

void MainWindow::axisLeftXChanged(double)
{
    //printf("left x %f\n", value);
}

void MainWindow::axisLeftYChanged(double value)
{
    printf("left y %f\n", -value);
    if (std::fabs(value) > 0.1f)
        m_commandData.leftThrottle = std::max(-1023, std::min(1023, int(-value * 1023.f)));
    else
        m_commandData.leftThrottle = 0;
}

void MainWindow::axisRightXChanged(double)
{
    //printf("right x %f\n", value);
}

void MainWindow::axisRightYChanged(double value)
{
    printf("right y %f\n", -value);
    if (std::fabs(value) > 0.1f)
        m_commandData.rightThrottle = std::max(-1023, std::min(1023, int(-value * 1023.f)));
    else
        m_commandData.rightThrottle = 0;
}

void MainWindow::connectionEstablished()
{
    ui->connectionLabel->setText("Connected to " + m_socket.getHostAddress());
}

void MainWindow::connectionLost()
{
    ui->connectionLabel->setText("Disconnected; reestablishing connection to raspberrypi.local");
}

void MainWindow::sendCommandPacket()
{
    m_socket.sendPacket(m_commandData);
}

MainWindow::~MainWindow()
{
}
