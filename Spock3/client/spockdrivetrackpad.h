#ifndef SPOCKDRIVETRACKPAD_H
#define SPOCKDRIVETRACKPAD_H

#include <QGroupBox>

class SpockDriveTrackpad : public QGroupBox
{
    Q_OBJECT
public:
    explicit SpockDriveTrackpad(QWidget *parent = Q_NULLPTR);
    ~SpockDriveTrackpad();
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);

signals:
    void _mousePressEvent(SpockDriveTrackpad *sender, QMouseEvent *event);
    void _mouseReleaseEvent(SpockDriveTrackpad *sender, QMouseEvent *event);
    void _mouseMoveEvent(SpockDriveTrackpad *sender, QMouseEvent* ev);
};

#endif // SPOCKDRIVETRACKPAD_H
