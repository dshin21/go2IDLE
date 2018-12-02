#include "go2idle.h"
#include "ui_go2idle.h"
#include <QDebug>
go2IDLE::go2IDLE(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::go2IDLE),
    io_thread(new IO(this))
{
    ui->setupUi(this);
    connect(ui->menu_select_file, &QAction::triggered, io_thread->get_file_handler(), &FileHandler::select_file);
    connect(ui->menu_disconnect, &QAction::triggered, this, &QWidget::close);
    //change this to send !start btn
    connect(ui->start_button, &QPushButton::pressed, io_thread, &IO::send_ENQ);
    //if data frame is received, print
    connect(io_thread, &IO::ready_to_print_signal, this, &go2IDLE::display_data);
}

go2IDLE::~go2IDLE()
{
    delete ui;
}


void go2IDLE::display_data(){
    QPlainTextEdit *text_edit = ui->console;
    text_edit->insertPlainText(io_thread->data_buffer);
}




