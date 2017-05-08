#ifndef SPOCKMOTORSTATUSBOX_H
#define SPOCKMOTORSTATUSBOX_H

#include <QGroupBox>
#include <memory>
#include "spockcommon.h"

namespace Ui {
class SpockMotorStatusBox;
}

class SpockMotorStatusBox : public QGroupBox
{
    Q_OBJECT

public:
    explicit SpockMotorStatusBox(QWidget *parent = 0);
    ~SpockMotorStatusBox();
    void setStatusData(const SpockMotorStatusData& data)
    {
        currentChanged(data.current * 0.125);
        battVChanged(data.batteryV * 0.05 + 4.0);
        tempChanged(data.temp * 0.645161290322581 - 50.0);
    }

signals:
    void currentChanged(double current);
    void battVChanged(double battV);
    void tempChanged(double temp);

private:
    std::unique_ptr<Ui::SpockMotorStatusBox> ui;
};

#endif // SPOCKMOTORSTATUSBOX_H
