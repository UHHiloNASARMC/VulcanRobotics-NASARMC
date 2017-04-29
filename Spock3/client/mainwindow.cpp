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
    connect(&m_devFinder, SIGNAL(buttonsChanged(int)), this, SLOT(buttonsChanged(int)));

    setFixedSize(870, 550);
    startTimer(20);
    m_devFinder.startScanning();

    m_cam0.setVideoOutput(ui->forwardVideo);
    m_cam0.setMedia(QUrl("udp://@127.0.0.1:48550"));
    m_cam0.play();
}

void MainWindow::timerEvent(QTimerEvent*)
{
    m_socket.reestablishConnection();
    sendCommandPacket();
    SpockStatusData data = m_socket.getStatusData();
    if (data.limitSwitches & 0x1)
        ui->armLimitLabel->setText("⬆");
    if (data.limitSwitches & 0x2)
        ui->armLimitLabel->setText("⬇");
    if (!data.limitSwitches)
        ui->armLimitLabel->setText("");
    ui->flMotorStat->setStatusData(data.frontLeftMotorStatus);
    ui->frMotorStat->setStatusData(data.frontRightMotorStatus);
    ui->blMotorStat->setStatusData(data.rearLeftMotorStatus);
    ui->brMotorStat->setStatusData(data.rearRightMotorStatus);
    ui->armMotorStat->setStatusData(data.armMotorStatus);
    ui->bucketMotorStat->setStatusData(data.bucketMotorStatus);

    printf("%d\n", m_cam0.mediaStatus());
}

void MainWindow::changeLeftThrottle(double value)
{
    m_commandData.leftThrottle = std::max(-1023, std::min(1023, int(value * 1023.f)));
    ui->leftThrottleLine->move(400, 170 - value * 140.0);
}

void MainWindow::changeRightThrottle(double value)
{
    m_commandData.rightThrottle = std::max(-1023, std::min(1023, int(value * 1023.f)));
    ui->rightThrottleLine->move(580, 170 - value * 140.0);
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
    changeLeftThrottle(leftMul * -y);
    changeRightThrottle(rightMul * -y);
    m_moveDragging = true;
    //printf("%d %d\n", m_commandData.leftThrottle, m_commandData.rightThrottle);
}

void MainWindow::driveRelease(SpockDriveTrackpad*, QMouseEvent*)
{
    //printf("release %f %f\n", ev->x() / float(sender->width()), ev->y() / float(sender->height()));
    changeLeftThrottle(0.0);
    changeRightThrottle(0.0);
    m_moveDragging = false;
}

void MainWindow::resetBucketButtons()
{
    ui->driveRadio->setDown(false);
    ui->driveRadio->setChecked(false);
    ui->scoopRadio->setDown(false);
    ui->scoopRadio->setChecked(false);
    ui->dumpRadio->setDown(false);
    ui->dumpRadio->setChecked(false);
    ui->weighButton->setDown(false);
}

void MainWindow::bucketDrive()
{
    resetBucketButtons();
    ui->driveRadio->setDown(true);
}

void MainWindow::bucketScoop()
{
    resetBucketButtons();
    ui->scoopRadio->setDown(true);
}

void MainWindow::bucketDump()
{
    resetBucketButtons();
    ui->dumpRadio->setDown(true);
}

void MainWindow::bucketWeigh()
{
    resetBucketButtons();
    ui->weighButton->setDown(true);
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
    if (m_moveDragging)
        return;
    //printf("left y %f\n", -value);
    if (std::fabs(value) > 0.1f)
        changeLeftThrottle(-value);
    else
        changeLeftThrottle(0.0);

}

void MainWindow::axisRightXChanged(double)
{
    //printf("right x %f\n", value);
}

void MainWindow::axisRightYChanged(double value)
{
    if (m_moveDragging)
        return;
    //printf("right y %f\n", -value);
    if (std::fabs(value) > 0.1f)
        changeRightThrottle(-value);
    else
        changeRightThrottle(0.0);
}

void MainWindow::buttonsChanged(int buttons)
{
    if ((m_lastButtons & 0x1000) == 0 && (buttons & 0x1000))
        ui->dumpRadio->click();
    if ((m_lastButtons & 0x4000) == 0 && (buttons & 0x4000))
        ui->scoopRadio->click();
    if ((m_lastButtons & 0x8000) == 0 && (buttons & 0x8000))
        ui->driveRadio->click();
    if ((m_lastButtons & 0x800) == 0 && (buttons & 0x800))
        ui->weighButton->click();
    m_lastButtons = buttons;
    //printf("%04X\n", buttons);
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
