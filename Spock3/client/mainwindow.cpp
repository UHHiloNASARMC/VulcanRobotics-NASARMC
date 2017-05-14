#define _USE_MATH_DEFINES
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMouseEvent>
#include <QtAV/QtAV.h>

#undef min
#undef max

// call this before loading the stream
static void tuneForZeroLatencyLiveStream(QtAV::AVPlayer& player)
{
    QVariantHash settings;

    // sets av_opt_set_xx and av_dict_set settings
    QVariantHash avFormat;

    {// these settings seem to have no impact on latency?
        //avFormat["fpsprobesize"] = 32;
        //avFormat["framerate"] = 100; // from: https://github.com/wang-bin/QtAV/issues/369
        //avFormat["rtbufsize"] = 512 * 1024;
        //avFormat["flush_packets"] = 1;
        //avFormat["avioflags"] = "direct";
        //avFormat["max_delay"] = 1000;
        //avFormat["fflags"] = "nobuffer";
        //avFormat["vprofile"] = "baseline";
    }

    avFormat["probesize"] = 1024; // great impact but how to calc the optimum?  // from: https://github.com/wang-bin/QtAV/wiki/FFmpeg-dict-opt-in-QtAV
    avFormat["tune"] = "zerolatency,fastdecode";
    {
        // is what zerolatency does:
        //  avFormat["bframes"] = 0;
        //  avFormat["force-cfr"] = 1;
        //  avFormat["no-mbtree"] = 1;
        //  avFormat["sync-lookahead"] = 0;
        //  avFormat["sliced-threads"] = 1;
        //  avFormat["rc-lookahead"] = 0;
    }

    settings["avformat"] = avFormat;
    player.setOptionsForFormat(avFormat);
}

static void startStream(QtAV::AVPlayer& player, QtAV::VideoRenderer* renderer, const char* url)
{
    player.masterClock()->setClockType(QtAV::AVClock::VideoClock);
    player.setBufferValue(1);
    tuneForZeroLatencyLiveStream(player);
    player.setRenderer(renderer);
    player.play(url);
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_socket(QStringLiteral("spock.local"), this)
{
    ui->setupUi(this);
    ui->connectionLabel->setText(QStringLiteral("Connecting to spock.local"));

    connect(&m_devFinder, SIGNAL(gamepadConnected()), this, SLOT(gamepadConnected()));
    connect(&m_devFinder, SIGNAL(gamepadDisconnected()), this, SLOT(gamepadDisconnected()));
    connect(&m_devFinder, SIGNAL(axisLeftXChanged(double)), this, SLOT(axisLeftXChanged(double)));
    connect(&m_devFinder, SIGNAL(axisLeftYChanged(double)), this, SLOT(axisLeftYChanged(double)));
    connect(&m_devFinder, SIGNAL(axisRightXChanged(double)), this, SLOT(axisRightXChanged(double)));
    connect(&m_devFinder, SIGNAL(axisRightYChanged(double)), this, SLOT(axisRightYChanged(double)));
    connect(&m_devFinder, SIGNAL(shoulderLeftChanged(double)), this, SLOT(shoulderLeftChanged(double)));
    connect(&m_devFinder, SIGNAL(shoulderRightChanged(double)), this, SLOT(shoulderRightChanged(double)));
    connect(&m_devFinder, SIGNAL(buttonsChanged(int)), this, SLOT(buttonsChanged(int)));

    setFixedSize(940, 550);
    startTimer(20);
    m_devFinder.startScanning();

    startStream(m_cam0, ui->forwardVideo, "udp://@239.0.0.10:5556");
    startStream(m_cam1, ui->reverseVideo, "udp://@239.0.0.10:5557");
    startStream(m_cam2, ui->bucketVideo, "udp://@239.0.0.10:5558");
    ui->forwardVideo->setVisible(true);
    ui->reverseVideo->setVisible(false);
}

static double PotCountsToRadians(int counts)
{
    return (-counts + 590.0) * M_PI / 700.0;
}

void MainWindow::timerEvent(QTimerEvent*)
{
    m_socket.reestablishConnection();
    if (m_canSend)
        sendCommandPacket();
    SpockStatusData data;
    std::tie(data, m_canSend) = m_socket.getStatusData();
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
    ui->armAngleNumber->display(PotCountsToRadians(data.armPot));
    ui->bucketAngleNumber->display(data.bucketPot);

    //resetBucketButtons();
    ui->scoopRadio->setChecked(false);
    ui->driveRadio->setChecked(false);
    ui->dumpRadio->setChecked(false);
    switch (data.sensedBucketState)
    {
    case ESpockBucketState::Scooping:
        ui->scoopRadio->setChecked(true);
        break;
    case ESpockBucketState::Driving:
        ui->driveRadio->setChecked(true);
        break;
    case ESpockBucketState::Dumping:
        ui->dumpRadio->setChecked(true);
        break;
    case ESpockBucketState::Weighing:
    default:
        break;
    }

    if (m_commandData.leftThrottle + m_commandData.rightThrottle > 0)
    {
        ui->forwardVideo->setVisible(true);
        ui->reverseVideo->setVisible(false);
    }
    if (m_commandData.leftThrottle + m_commandData.rightThrottle < 0)
    {
        ui->forwardVideo->setVisible(false);
        ui->reverseVideo->setVisible(true);
    }
}

void MainWindow::changeLeftThrottle(double value)
{
    m_commandData.leftThrottle = std::max(-1023, std::min(1023, int(value * 1023.f)));
    ui->leftThrottleLine->move(400, 100 - value * 70.0);
}

void MainWindow::changeRightThrottle(double value)
{
    m_commandData.rightThrottle = std::max(-1023, std::min(1023, int(value * 1023.f)));
    ui->rightThrottleLine->move(580, 100 - value * 70.0);
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
    float leftMul = std::max(0.f, std::min(1.f - x, 1.f)) * 2.f - 1.f;
    float rightMul = std::max(0.f, std::min(x + 1.f, 1.f)) * 2.f - 1.f;
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
    m_commandData.bucketState = ESpockBucketState::Driving;
}

void MainWindow::bucketScoop()
{
    resetBucketButtons();
    ui->scoopRadio->setDown(true);
    m_commandData.bucketState = ESpockBucketState::Scooping;
}

void MainWindow::bucketDump()
{
    resetBucketButtons();
    ui->dumpRadio->setDown(true);
    m_commandData.bucketState = ESpockBucketState::Dumping;
}

void MainWindow::bucketWeigh()
{
    resetBucketButtons();
    ui->weighButton->setDown(true);
}

void MainWindow::bucketPanic()
{
    resetBucketButtons();
    m_commandData.bucketState = ESpockBucketState::Invalid;
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

void MainWindow::shoulderLeftChanged(double value)
{
    //printf("Left Moved %f\n", value);
}

void MainWindow::shoulderRightChanged(double value)
{
    if (!ui->armSlider->isSliderDown())
        ui->armSlider->setValue(int(value) * 4);
    //printf("Right Moved %f\n", value);
}

void MainWindow::buttonsChanged(int buttons)
{
    if ((m_lastButtons & 0x1000) == 0 && (buttons & 0x1000))
        ui->dumpRadio->click();
    if ((m_lastButtons & 0x2000) == 0 && (buttons & 0x2000))
        ui->panicButton->click();
    if ((m_lastButtons & 0x4000) == 0 && (buttons & 0x4000))
        ui->scoopRadio->click();
    if ((m_lastButtons & 0x8000) == 0 && (buttons & 0x8000))
        ui->driveRadio->click();
    if ((m_lastButtons & 0x800) == 0 && (buttons & 0x800))
        ui->weighButton->click();
    if (!ui->bucketSlider->isSliderDown())
    {
        if (buttons & 0x0100)
            ui->bucketSlider->setValue(-512);
        if (buttons & 0x0400)
            ui->bucketSlider->setValue(512);
        if ((buttons & 0x0500) == 0 || (buttons & 0x0500) == 0x0500)
            ui->bucketSlider->setValue(0);
    }
    m_lastButtons = buttons;
    //printf("%04X\n", buttons);
}

void MainWindow::connectionEstablished()
{
    ui->connectionLabel->setText("Connected to " + m_socket.getHostAddress());
    m_canSend = true;
}

void MainWindow::connectionLost()
{
    ui->connectionLabel->setText("Disconnected; reestablishing connection to spock.local");
    m_canSend = false;
}

void MainWindow::doReconnect()
{
    m_socket.forceReconnect();
    m_canSend = true;
}

void MainWindow::sendCommandPacket()
{
    m_socket.sendPacket(m_commandData);
}

MainWindow::~MainWindow()
{
    m_cam0.stop();
    m_cam1.stop();
}
