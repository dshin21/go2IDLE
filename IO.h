#ifndef IO_H
#define IO_H

#include <QSerialPort>
#include <QByteArray>
#include <QThread>
#include "CRC.h"
#include "constants.h"


class IO : public QThread
{
private:
    QSerialPort* serial_port;

public:

    IO();
    void send_EOT();
    void send_ENQ();
    void send_ACK();
    void send_NAK();
    void send_DATA_FRAME();
    QByteArray make_frame(const QByteArray& data);

public slots:
    void write_to_port(const QByteArray &data);

};

#endif // IO_H
