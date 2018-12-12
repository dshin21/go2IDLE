/*------------------------------------------------------------------------------------------------------------------
--SOURCE FILE:  go2IDLE.cpp-   UI application that connects signals updates from IO.cpp to display
--
--PROGRAM:      Radio Modem Protocol Driver
--
--FUNCTIONS:
--              void display_data();
--              void update_ack();
--              void update_nak();
--              void update_frame();
--              void update_beRate();
--
--DATE:         Nov. 21, 2018 Daniel Shin - first draft
--
--REVISIONS:    Dec. 11, 2018 Simon Chen  - added update functions
--
--DESIGNER:     Daniel Shin
--
--PROGRAMMER:   Daniel Shin, Simon Chen
--
--NOTES:
--This is the UI part of the program, it shows statistics of transmission activity by IO.cpp.
----------------------------------------------------------------------------------------------------------------------*/
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

/*------------------------------------------------------------------------------------------------------------------
--FUNCTION:     display_data
--
--DATE:         Nov. 22, 2018   Daniel Shin - first draft
--
--DESIGNER:     Daniel Shin
--
--PROGRAMMER:   Daniel Shin
--
--INTERFACE:    void display_data(void)
--
--RETURNS:      void
--
--NOTES:
--This function displays data to window when receiving an ready_to_print_signal event.
----------------------------------------------------------------------------------------------------------------------*/
void go2IDLE::display_data(){
    QPlainTextEdit *text_edit = ui->console;
    text_edit->insertPlainText(io_thread->data_buffer);

}

/*------------------------------------------------------------------------------------------------------------------
--FUNCTION:     update_ack
--
--DATE:         Dec. 11, 2018   Simon Chen - first draft
--
--DESIGNER:     Allan Hsu
--
--PROGRAMMER:   Simon Chen
--
--INTERFACE:    void update_ack(void)
--
--RETURNS:      void
--
--NOTES:
--This function updates counts of ACKs when triggered by ack_sent_signal_statistic event.
----------------------------------------------------------------------------------------------------------------------*/
void go2IDLE::update_ack(){
   ackCount++;
   ui->label_acks->setText("Acks: " + QString::number(ackCount));

}

/*------------------------------------------------------------------------------------------------------------------
--FUNCTION:     update_nak
--
--DATE:         Dec. 11, 2018   Simon Chen - first draft
--
--DESIGNER:     Allan Hsu
--
--PROGRAMMER:   Simon Chen
--
--INTERFACE:    void update_nak(void)
--
--RETURNS:      void
--
--NOTES:
--This function updates counts of NAKs when triggered by nak_sent_signal_statistic event.
----------------------------------------------------------------------------------------------------------------------*/
void go2IDLE::update_nak(){
 nakCount++;
 ui->label_naks->setText("Naks: " + QString::number(nakCount));

}

/*------------------------------------------------------------------------------------------------------------------
--FUNCTION:     update_frame
--
--DATE:         Dec. 11, 2018   Simon Chen - first draft
--
--DESIGNER:     Allan Hsu
--
--PROGRAMMER:   Simon Chen
--
--INTERFACE:    void update_frame(void)
--
--RETURNS:      void
--
--NOTES:
--This function updates counts of frames when triggered by frame_sent_signal_statistic event.
----------------------------------------------------------------------------------------------------------------------*/
void go2IDLE::update_frame(){
  frameCount++;
   ui->label_packets_transferred->setText("Packets Transfered: " + QString::number(frameCount));

}

/*------------------------------------------------------------------------------------------------------------------
--FUNCTION:     update_beRate
--
--DATE:         Dec. 11, 2018   Simon Chen - first draft
--
--DESIGNER:     Allan Hsu
--
--PROGRAMMER:   Simon Chen
--
--INTERFACE:    void update_beRate(void)
--
--RETURNS:      void
--
--NOTES:
--This function updates counts of beRate when triggered by ...
----------------------------------------------------------------------------------------------------------------------*/
void go2IDLE::update_beRate(){
   beRateCount++;
   ui->label_bit_error_rate->setText("BE Rate:  " + QString::number(beRateCount));

}
