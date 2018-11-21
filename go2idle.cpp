#include "go2idle.h"
#include "ui_go2idle.h"

go2IDLE::go2IDLE(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::go2IDLE)
{
    ui->setupUi(this);
}

go2IDLE::~go2IDLE()
{
    delete ui;
}
