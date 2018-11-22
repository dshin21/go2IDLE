#include "go2idle.h"
#include "ui_go2idle.h"

go2IDLE::go2IDLE(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::go2IDLE)
{
    ui->setupUi(this);
    connect(ui->menu_select_file, &QAction::triggered, this, &go2IDLE::select_file);
}

go2IDLE::~go2IDLE()
{
    delete ui;
}

void go2IDLE::select_file(){
    QString fileName = QFileDialog::getOpenFileName(
                this,
                tr("Select a File to Send"),
                "",
                tr("Text File (*.txt)")
                );
}
