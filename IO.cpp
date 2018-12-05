#include "IO.h"
#include <QDebug>
#include <algorithm>

IO::IO(QObject *parent)
    : QThread(parent),
      serial_port(new QSerialPort(this)),
      master_buffer(QByteArray()),
      file_handler(new FileHandler(this)),
      is_processed(false)
{
    init_port();
    connect(serial_port, &QSerialPort::readyRead, this, &IO::read_from_port, Qt::QueuedConnection);
    connect(this, &IO::write_to_port_signal, this, &IO::write_to_port, Qt::QueuedConnection);
    //if incoming frame, process them
    connect(this, &IO::data_received_signal, this, &IO::process_frames);

    qsrand(0);
    ENQ_backoff_Timer = new QTimer(this);
    backingOff = false;
    connect(ENQ_backoff_Timer, &QTimer::timeout, this, &IO::send_ENQ_after_backoff);
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
    emit write_to_port_signal(EOT_FRAME);
}

void IO::send_ENQ()
{
    CURRENT_STATE = REQUEST_LINE;
    emit write_to_port_signal(ENQ_FRAME);

}

void IO::send_ENQ_after_backoff(){
    if(backingOff){
        emit write_to_port_signal(ENQ_FRAME);
        backingOff = false;
    }
}

void IO::send_ACK()
{
    emit write_to_port_signal(ACK_FRAME);
}

void IO::send_NAK()
{
    emit write_to_port_signal(NAK_FRAME);
}

void IO::send_DATA_FRAME()
{
    emit write_to_port_signal(make_frame(file_handler->get_next()));
}

QByteArray IO::make_frame(const QByteArray &data)
{
    QByteArray padding = QByteArray(DATA_LENGTH - data.size(), 0x0);
    uint32_t crc = CRC::Calculate(padding.data(), DATA_LENGTH, CRC::CRC_32());
    //TODO: alternate DC1
    QByteArray frame = SYN_FRAME + DC1_FRAME + data + padding;
    frame.push_back(crc);

    return frame;
}

void IO::read_from_port()
{
    master_buffer.clear();
    master_buffer += serial_port->readAll();
    frame += master_buffer;
    //qDebug()<<frame;
    emit data_received_signal(frame);

}

void IO::handle_control_buffer()
{
    //char after SYN
    if((control_buffer[1] == (char)DC1 || control_buffer[1] == (char)DC2) && control_buffer.size() != 3){
        qDebug() << "its a data frame";
        emit ready_to_print_signal();
    }else{
        //its a control frame
        switch (control_buffer.at(1)) {
        case DC1:
            qDebug() << "DC1 received";
            break;
        case DC2:
            qDebug() << "DC2 received";
            break;
        case EOT:
            qDebug() << "EOT received";
            break;
        case ENQ:
            received_ENQ();
            break;
        case ACK:
            qDebug() << "ACK received";
            break;
        case NAK:
            qDebug() << "NAK received";
            break;
        default:
            break;
        }
        qDebug() << "its a control frame";
    }
}


void IO::received_ENQ(){
    switch(CURRENT_STATE){
        case IDLE:
            send_ACK();
            break;
        case REQUEST_LINE:
            if(ENQ_backoff_Timer->isActive() && backingOff == true){
                send_ACK();
                CURRENT_STATE = RECEIVE_FRAME;
                backingOff = false;
            }
            if(ENQ_backoff_Timer->isActive() == false){

                ENQ_backoff_Timer->start(qrand() % 500);
                backingOff = true;
            }

            break;
        case SEND_STATE:
            //do nothing
            break;
        case WAIT_RESPONSE:
            //do nothing
            break;
        case RESEND_FRAME:
            //do nothing
            break;
        case RECEIVE_FRAME:
         //do nothing
            break;
        default:
            break;
    }
}


void IO::received_EOT(){
    switch(CURRENT_STATE){
        case IDLE:
            qDebug() << "DC1 received";
            break;
        case REQUEST_LINE:
            qDebug() << "DC2 received";
            break;
        case SEND_STATE:
            qDebug() << "EOT received";
            break;
        case WAIT_RESPONSE:
            qDebug() << "ENQ received";
            break;
        case RESEND_FRAME:
            qDebug() << "ACK received";
            break;
        default:
            break;
    }
}


void IO::received_NAK(){
    switch(CURRENT_STATE){
        case IDLE:
            qDebug() << "DC1 received";
            break;
        case REQUEST_LINE:
            qDebug() << "DC2 received";
            break;
        case SEND_STATE:
            qDebug() << "EOT received";
            break;
        case WAIT_RESPONSE:
            qDebug() << "ENQ received";
            break;
        case RESEND_FRAME:
            qDebug() << "ACK received";
            break;
        default:
            break;
    }
}


void IO::received_ACK(){
    switch(CURRENT_STATE){
        case IDLE:
            qDebug() << "DC1 received";
            break;
        case REQUEST_LINE:
            qDebug() << "DC2 received";
            break;
        case SEND_STATE:
            qDebug() << "EOT received";
            break;
        case WAIT_RESPONSE:
            qDebug() << "ENQ received";
            break;
        case RESEND_FRAME:
            qDebug() << "ACK received";
            break;
        default:
            break;
    }
}


void IO::process_frames(QString data){

    data_buffer = "";
    control_buffer.clear();

    if(data[0] == (char)SYN
            && (data[2] == (char)DC1 || data[2] == (char)DC2)){

        control_buffer = data.toUtf8();
        qDebug() << "it's a control frame!";
        qDebug() << data;
         frame.clear();
        handle_control_buffer();
    } else {
       if(frame.size() == 1024){
           data_buffer = data;
           qDebug() << "it's a data frame!";
           qDebug() << data;
           //check crc
           frame.clear();
       }
    }

//    for(int i = 0; i < data.length(); ++i){
//        QChar current_char = data.at(i);
//        if(!current_char.isNull()
//                && (current_char.isLetterOrNumber()
//                    || current_char.isSpace()
//                    || current_char.isPunct())){
//            data_buffer += current_char;
//        }else{
//          control_buffer+= (current_char);

//        }
//    }
   // handle_control_buffer();

}

//TODO:
// - timeouts
// - ENQ and ACKs
