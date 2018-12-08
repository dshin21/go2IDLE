#ifndef IO_H
#define IO_H

#include <QSerialPort>
#include <QByteArray>
#include <QThread>
#include <vector>
#include <QTimer>
#include "CRC.h"
#include "constants.h"

#include "filehandler.h"

class IO : public QThread
{
    Q_OBJECT
private:
    QSerialPort* serial_port;
    FileHandler* file_handler;
    QTimer* ENQ_backoff_Timer;
    QTimer* IDLE_EOT_timer;
   //QTimer* retransmission_timeout;
    bool backingOff;
    int resend_counts;
    QThread idle_eot_thread;


public:
    bool is_processed;
    QByteArray master_buffer;
    QByteArray frame;
    QString data_buffer;
    QByteArray control_buffer;
    IO(QObject *parent);
    void send_EOT();
    void send_ENQ();
    void send_ACK();
    void send_NAK();
    void send_DATA_FRAME();
    void resend_DATA_FRAME();
    QByteArray make_frame(const QByteArray& data);

    void handle_control_buffer();

    inline FileHandler* get_file_handler() const {return file_handler;}

    void received_ENQ();
    void received_EOT();
    void received_NAK();
    void received_ACK();
    //void idle_sent_eot();

public slots:
    void init_port();
    void write_to_port(const QByteArray &data);
    void read_from_port();
    void process_frames(QString data);
    void send_ENQ_after_backoff();
    void resend_frame();
    void create_IDLE_send_EOT();

signals:
    void write_to_port_signal(const QByteArray &frame);
    void data_received_signal(QString data);
    void ready_to_print_signal();
};

#endif // IO_H
