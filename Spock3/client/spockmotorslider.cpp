#include "spockmotorslider.h"

SpockMotorSlider::SpockMotorSlider(QWidget *parent)
    : QSlider(parent)
{

}

void SpockMotorSlider::mouseReleaseEvent(QMouseEvent *)
{
    setValue(0);
    setSliderDown(false);
}
