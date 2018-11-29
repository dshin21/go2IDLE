#include "IO.h"

IO::IO(QObject *parent)
    : QThread(parent),
      serial_port(new QSerialPort(this)),
      buffer(QByteArray()),
      file_handler(new FileHandler(this))
{
    init_port();
    connect(serial_port, &QSerialPort::readyRead, this, &IO::read_from_port, Qt::QueuedConnection);
    connect(this, &IO::write_to_port_signal, this, &IO::write_to_port, Qt::QueuedConnection);
}

void IO::init_port(){
    serial_port->close();
    serial_port->setPortName("COM1");
    serial_port->open(QIODevice::ReadWrite);
    serial_port->setBaudRate(QSerialPort::Baud9600);
    serial_port->setDataBits(QSerialPort::Data8);
    serial_port->setParity(QSerialPort::NoParity);
    serial_port->setStopBits(QSerialPort::OneStop);
    serial_port->setFlowControl(QSerialPort::NoFlowControl);
}

void IO::write_to_port(const QByteArray &data)
{
    serial_port->write(data);
    serial_port->flush();
}

void IO::send_EOT()
{
    emit write_to_port_signal(QByteArray(1,EOT));
}

void IO::send_ENQ()
{
    emit write_to_port_signal(QByteArray(1,ENQ));
}

void IO::send_ACK()
{
    emit write_to_port_signal(QByteArray(1,ACK));
}

void IO::send_NAK()
{
    emit write_to_port_signal(QByteArray(1,NAK));
}

void IO::send_DATA_FRAME()
{
    emit write_to_port_signal(make_frame(file_handler->get_next()));
}

QByteArray IO::make_frame(const QByteArray &data)
{
    QByteArray padding = QByteArray(DATA_LENGTH - data.size(), 0x0);
    uint32_t crc = CRC::Calculate(padding.data(), DATA_LENGTH, CRC::CRC_32());
    QByteArray frame = SYN_FRAME + DC1_FRAME + data + padding;
    frame.push_back(crc);
    return frame;
}

void IO::read_from_port()
{
    buffer += serial_port->readAll();
    emit data_received_signal(buffer);
}

void IO::handleBuffer()
{
    if(buffer.contains(ENQ_FRAME)){

    }
}

//TODO:
// - decapsulate data frame
// - timeouts
// - ENQ and ACKs
