#include "spockdrivetrackpad.h"
#include <QMouseEvent>

SpockDriveTrackpad::SpockDriveTrackpad(QWidget *parent)
    : QGroupBox(parent)
{
}

SpockDriveTrackpad::~SpockDriveTrackpad() {}

void SpockDriveTrackpad::mousePressEvent(QMouseEvent *event)
{
    emit _mousePressEvent(this, event);
}

void SpockDriveTrackpad::mouseReleaseEvent(QMouseEvent *event)
{
    emit _mouseReleaseEvent(this, event);
}

void SpockDriveTrackpad::mouseMoveEvent(QMouseEvent *event)
{
    emit _mouseMoveEvent(this, event);
}
