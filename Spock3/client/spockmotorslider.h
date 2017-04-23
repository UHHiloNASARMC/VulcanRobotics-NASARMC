#ifndef SPOCKMOTORSLIDER_H
#define SPOCKMOTORSLIDER_H

#include <QSlider>

class SpockMotorSlider : public QSlider
{
public:
    explicit SpockMotorSlider(QWidget *parent = Q_NULLPTR);
    void mouseReleaseEvent(QMouseEvent *ev);
};

#endif // SPOCKMOTORSLIDER_H
