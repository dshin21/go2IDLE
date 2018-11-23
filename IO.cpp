#include "IO.h"

IO::IO() : serial_port(new QSerialPort(this))
{

}

void IO::write_to_port(const QByteArray &data)
{
    serial_port->write(data);
    serial_port->flush();
}

void IO::send_EOT()
{
    emit write_to_port(QByteArray(1,EOT));
}

void IO::send_ENQ()
{
    emit write_to_port(QByteArray(1,ENQ));
}

void IO::send_ACK()
{
    emit write_to_port(QByteArray(1,ACK));
}

void IO::send_NAK()
{
    emit write_to_port(QByteArray(1,NAK));
}

void IO::send_DATA_FRAME()
{

}

QByteArray IO::make_frame(const QByteArray &data)
{
    QByteArray padding = QByteArray(DATA_LENGTH - data.size(), 0x0);
    uint32_t crc = CRC::Calculate(padding.data(), DATA_LENGTH, CRC::CRC_32());
    QByteArray frame = SYN_FRAME + DC1_FRAME + data + padding;
    frame.push_back(crc);
    return frame;
}
