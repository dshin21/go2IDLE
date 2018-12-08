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
    retransmission_Timer = new QTimer(this);
    data_frame_receive_Timer = new QTimer(this);
    resend_counts = 0;
    backingOff = false;
    connect(ENQ_backoff_Timer, &QTimer::timeout, this, &IO::send_ENQ_after_backoff);
    connect(retransmission_Timer, &QTimer::timeout, this, &IO::resend_frame);
    connect(data_frame_receive_Timer, &QTimer::timeout, this, &IO::receive_timeout);

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

void IO::resend_frame(){
    if(resend_counts < MAX_RESENDS){
        //retransmission_timeout->start(TRANSMISSION_TIMEOUT);
        resend_DATA_FRAME();
    }else{
        CURRENT_STATE = IDLE;
        resend_counts = 0;
    }
}

void IO::send_ACK()
{
    CURRENT_STATE = RECEIVE_FRAME;
    data_frame_receive_Timer->start(TRANSMISSION_TIMEOUT);
    emit write_to_port_signal(ACK_FRAME);
}

void IO::send_NAK()
{
    CURRENT_STATE = RESEND_FRAME;
    data_frame_receive_Timer->start(TRANSMISSION_TIMEOUT);
    emit write_to_port_signal(NAK_FRAME);
}

void IO::send_DATA_FRAME()
{
    QByteArray temp;
    temp = make_frame(file_handler->get_next());
    if(temp.at(1) != EOT){
        retransmission_Timer->start(TRANSMISSION_TIMEOUT);
    }
    emit write_to_port_signal(temp);

}

void IO::resend_DATA_FRAME()
{
    if(resend_counts > 3){
        CURRENT_STATE = IDLE;
    } else {
        resend_counts ++;
        retransmission_Timer->start(TRANSMISSION_TIMEOUT);
        emit write_to_port_signal(make_frame(file_handler->get_prev()));
    }
    //retransmission_timeout->start(TRANSMISSION_TIMEOUT);

}

void IO::receive_timeout(){
    CURRENT_STATE = IDLE;
}



QByteArray IO::make_frame(const QByteArray &data)
{

    if(data.isEmpty()){
        CURRENT_STATE = IDLE;

         return EOT_FRAME;

    } else {
        if(data_frame_receive_Timer->isActive()){
            data_frame_receive_Timer->stop();
        }
        QByteArray padding = QByteArray(DATA_LENGTH - data.size(), 0x0);
        uint32_t crc = CRC::Calculate(padding.data(), DATA_LENGTH, CRC::CRC_32());
        //TODO: alternate DC1
        QByteArray frame;
        if (dcFlip == false) {
            frame = SYN_FRAME + DC1_FRAME + data + padding;
            qDebug()<<"sent dc1";
        } else {
           frame = SYN_FRAME + DC2_FRAME + data + padding;
            qDebug()<<"sent dc2";
        }
        frame.push_back(crc);
        return frame;
    }



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

    //its a control frame
    switch (control_buffer[1]) {
        case DC1:
            qDebug() << "DC1 received";
            break;
        case DC2:
            qDebug() << "DC2 received";
            break;
        case EOT:
            received_EOT();
            break;
        case ENQ:
            received_ENQ();
            break;
        case ACK:
            received_ACK();
            break;
        case NAK:
            received_NAK();
            break;
        default:
            break;
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
                qDebug()<<"send ack";
            }
            if(ENQ_backoff_Timer->isActive() == false){

                ENQ_backoff_Timer->start(qrand() % 500);
                backingOff = true;
                qDebug()<<"backoff";
            }

            break;
        case SEND_STATE:
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
            send_EOT();
            break;
        case REQUEST_LINE:
            CURRENT_STATE = SEND_STATE;
            send_ACK();
            break;
        case SEND_STATE:
            CURRENT_STATE = IDLE;
            //retransmission_timeout->stop();
            break;
        case RESEND_FRAME:
            break;
        case RECEIVE_FRAME:
            CURRENT_STATE = IDLE;
            qDebug() <<"return to idle from receive xxxxxxxxxxxxxxxxxxxxxx";
            break;
        default:
            break;
    }
}


void IO::received_NAK(){
    switch(CURRENT_STATE){
        case IDLE:
            break;
        case REQUEST_LINE:
            break;
        case SEND_STATE:
            CURRENT_STATE = SEND_STATE;
            if(retransmission_Timer->isActive()){
                retransmission_Timer->stop();
                qDebug()<<"received NAK stopping timer";
            }
            resend_counts = 0;
            resend_DATA_FRAME();
            break;
        case RECEIVE_FRAME:
            break;
        case RESEND_FRAME:
            break;
        default:
            break;
    }
}


void IO::received_ACK(){


    switch(CURRENT_STATE){
        case IDLE:
            break;
        case REQUEST_LINE:
            CURRENT_STATE = SEND_STATE;
            resend_counts = 0;
            send_DATA_FRAME();
            break;
        case SEND_STATE:
            CURRENT_STATE = SEND_STATE;
            if(retransmission_Timer->isActive()){
                retransmission_Timer->stop();
                qDebug()<<"received ACK stopping timer";
            }
            //retransmission_timeout->stop();
            resend_counts = 0;
            dcFlip = !dcFlip;

            send_DATA_FRAME();
            break;
        case RECEIVE_FRAME:

            break;
        case RESEND_FRAME:

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
        qDebug()<<"entered process frame if " <<frame.size();
        control_buffer = data.toUtf8();
        qDebug() << "it's a control frame!";
        qDebug() << data;
        frame.clear();
        handle_control_buffer();
    } else {
        qDebug()<<"entered process frame else " <<frame.size();
       if(frame.size() == 1024){
           if (dcFlipReceive == false&& frame.at(1) == DC1) {//dc1
              data_buffer = data;
              qDebug() << "it's a data frame with DC2!";
              qDebug() << data;
              dcFlipReceive = !dcFlipReceive;
              //check crc
              send_ACK();
              frame.clear();
           } else if (dcFlipReceive == true && frame.at(1) == DC2) {//dc1
               data_buffer = data;
               qDebug() << "it's a data frame with DC2!";
               qDebug() << data;
               dcFlipReceive = !dcFlipReceive;
               //check crc
               send_ACK();
               frame.clear();
            } else {
               //do nothing
               //add timeout
           }


//           send_NAK();

       }
    }

}

//TODO:
// timeouts
// check CRC
// Deal with exceeding maximum frames 50
// Idle mode send EOTs
// connection timeout (almost done) Allan Hsu
