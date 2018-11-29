#include "go2idle.h"
#include "ui_go2idle.h"

go2IDLE::go2IDLE(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::go2IDLE),
    io_thread(new IO(this))
{
    ui->setupUi(this);
    connect(ui->menu_select_file, &QAction::triggered, io_thread->get_file_handler(), &FileHandler::select_file);
    connect(ui->menu_disconnect, &QAction::triggered, this, &QWidget::close);
    connect(io_thread, &IO::data_received_signal, this, &go2IDLE::display_data);
    //change this to send enq
    connect(ui->start_button, &QPushButton::pressed, io_thread, &IO::send_DATA_FRAME);
}

go2IDLE::~go2IDLE()
{
    delete ui;
}


void go2IDLE::display_data(const QString data){
    QPlainTextEdit *text_edit = ui->console;
    text_edit->insertPlainText(data);
}
