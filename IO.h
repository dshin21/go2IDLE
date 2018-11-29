#ifndef IO_H
#define IO_H

#include <QSerialPort>
#include <QByteArray>
#include <QThread>
#include "CRC.h"
#include "constants.h"

#include "filehandler.h"

class IO : public QThread
{
    Q_OBJECT
private:
    QSerialPort* serial_port;
    FileHandler* file_handler;

public:
    QByteArray buffer;
    IO(QObject *parent);
    void send_EOT();
    void send_ENQ();
    void send_ACK();
    void send_NAK();
    void send_DATA_FRAME();
    QByteArray make_frame(const QByteArray& data);

    void handleBuffer();

    inline FileHandler* get_file_handler() const {return file_handler;}

public slots:
    void init_port();
    void write_to_port(const QByteArray &data);
    void read_from_port();


signals:
    void write_to_port_signal(const QByteArray &frame);
    void data_received_signal(const QString data);
};

#endif // IO_H
