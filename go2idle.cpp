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
    connect(io_thread, &IO::ack_sent_signal_statistic, this, &go2IDLE::update_ack);
     connect(io_thread, &IO::nak_sent_signal_statistic, this, &go2IDLE::update_nak);
      connect(io_thread, &IO::frame_sent_signal_statistic, this, &go2IDLE::update_frame);

    //ui->label_acks->setText("Acks 0 " + NUM_ACK);
}

go2IDLE::~go2IDLE()
{
    delete ui;
}

void go2IDLE::display_data(){
    QPlainTextEdit *text_edit = ui->console;
    text_edit->insertPlainText(io_thread->data_buffer);

}
int ackCount = 0;
int nakCount = 0;
int frameCount = 0;
int beRate = 0;

void go2IDLE::update_ack(){
   ackCount++;
   ui->label_acks->setText("Acks: " + QString::number(ackCount));

}

void go2IDLE::update_nak(){
 nakCount++;
 ui->label_naks->setText("Naks: " + QString::number(nakCount));

}

void go2IDLE::update_frame(){
  frameCount++;
   ui->label_packets_transferred->setText("Packets Transfered: " + QString::number(frameCount));

}

void go2IDLE::update_beRate(){
   beRateCount++;
   ui->label_bit_error_rate->setText("BE Rate:  " + QString::number(beRateCount));

}




