#include "spockmotorstatusbox.h"
#include "ui_spockmotorstatusbox.h"

SpockMotorStatusBox::SpockMotorStatusBox(QWidget *parent) :
    QGroupBox(parent),
    ui(new Ui::SpockMotorStatusBox)
{
    ui->setupUi(this);
    setLayout(ui->gridLayout);
}

SpockMotorStatusBox::~SpockMotorStatusBox()
{
}
