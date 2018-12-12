/*------------------------------------------------------------------------------------------------------------------
--SOURCE FILE:  IO.cpp-   An application that will read write from serial port and save file.
--
--PROGRAM:      Radio Modem Protocol Driver
--
--FUNCTIONS:
--              void IO::init_port()
--              void IO::write_to_port(const QByteArray &data)
--              void IO::send_EOT()
--              void IO::send_ENQ()
--              void IO::send_ENQ_after_backoff()
--              void IO::resend_frame()
--              void IO::IDLE_send_EOT()
--              void IO::IDLE_EOT_not_received()
--              void IO::terminate_program()
--              void IO::send_ACK()
--              void IO::send_NAK()
--              void IO::send_DATA_FRAME()
--              void IO::resend_DATA_FRAME()
--              void IO::receive_timeout()
--              QByteArray IO::make_frame(const QByteArray &data)
--              void IO::read_from_port()
--              void IO::handle_control_buffer()
--              void IO::received_ENQ()
--              void IO::received_EOT()
--              void IO::received_NAK()
--              void IO::received_ACK()
--              void IO::process_frames(QString data)
--
--
--DATE:         Nov 22, 2018    Daniel Shin - skeleton
--
--REVISIONS:    Nov 22, 2018    Simon Chen  - send_ENQ_after_backoff added
                Nov 28, 2018    Daniel Shin - added initport()
                Nov 28, 2018    Simon Chen  - send_ENQ_after_backoff added
                Dec. 2, 2018    Daniel Shin - modified make_frame
                Dec. 2, 2018    Jenny Ly    - modified handle_control_buffer, receive_ACK...
                Dec. 5, 2018    Allan Hsu   - added timer prototype
                Dec. 5, 2018    Simon Chen  - resend_DATA_FRAME dded
                Dec. 7, 2018    Daniel Shin - IDLE_send_EOT, terminate program added
                Dec.11, 2018    Allan Hsu   - modified process frames
--
--DESIGNER:     Daniel Shin
--
--PROGRAMMER:   Daniel Shin, Simon Chen, Allan Hsu, Jenny Ly
--
--NOTES:
--The program will read from and write to serial port. The design utilizes Qt's serial port class, signal to slots
--mechanism to be event driven. Once data is ready to be read from port, it will read and process the frames received
--and act according to the send and wait protocol
----------------------------------------------------------------------------------------------------------------------*/
#include "IO.h"
#include <QDebug>
#include <algorithm>

IO::IO(QObject *parent)
    : QThread(parent),
      serial_port(new QSerialPort(this)),
      master_buffer(QByteArray()),
      file_handler(new FileHandler(this)),
      is_processed(false),
      IDLE_EOT_send_timer(new QTimer(this)),
      IDLE_EOT_received_timer(new QTimer(this))
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

    IDLE_EOT_send_timer->start(500);
    connect(IDLE_EOT_send_timer, &QTimer::timeout, this, &IO::IDLE_send_EOT);
    IDLE_EOT_received_timer->start(30000);
    connect(IDLE_EOT_received_timer, &QTimer::timeout, this, &IO::IDLE_EOT_not_received);
    connect(retransmission_Timer, &QTimer::timeout, this, &IO::resend_frame);
    connect(data_frame_receive_Timer, &QTimer::timeout, this, &IO::receive_timeout);
}

/*------------------------------------------------------------------------------------------------------------------
--FUNCTION:     init_port
--
--DATE:         Nov 28, 2018    Daniel Shin - created
--
--DESIGNER:     Daniel Shin
--
--PROGRAMMER:   Daniel Shin
--
--INTERFACE:    void init_port (void)
--
--RETURNS:      void.
--
--NOTES:
--This function initialize the default serial port settings and connects to the serial port.
----------------------------------------------------------------------------------------------------------------------*/
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

/*------------------------------------------------------------------------------------------------------------------
--FUNCTION:     write_to_port
--
--DATE:         Nov 22, 2018    Daniel Shin - created
--
--DESIGNER:     Daniel Shin
--
--PROGRAMMER:   Daniel Shin
--
--INTERFACE:    void write_to_port (const QByteArray &data)
--                          const QByteArray &data: the data to be written to serial port
--RETURNS:      void.
--
--NOTES:
--This function writes to serial port and flushes afterwards.
----------------------------------------------------------------------------------------------------------------------*/
void IO::write_to_port(const QByteArray &data)
{
    serial_port->write(data);
    serial_port->flush();
}

/*------------------------------------------------------------------------------------------------------------------
--FUNCTION:     send_EOT
--
--DATE:         Nov 22, 2018
--
--DESIGNER:     Daniel Shin
--
--PROGRAMMER:   Daniel Shin
--
--INTERFACE:    void send_EOT(void)
--
--RETURNS:      void.
--
--NOTES:
--This function emits the signal acting as an event signal to send EOT.
----------------------------------------------------------------------------------------------------------------------*/
void IO::send_EOT()
{
    emit write_to_port_signal(EOT_FRAME);
}

/*------------------------------------------------------------------------------------------------------------------
--FUNCTION:     send_ENQ
--
--DATE:         Nov 22, 2018
--
--DESIGNER:     Daniel Shin
--
--PROGRAMMER:   Daniel Shin
--
--INTERFACE:    void send_ENQ(void)
--
--RETURNS:      void.
--
--NOTES:
--This function emits the signal acting as an event signal to send ENQ.
----------------------------------------------------------------------------------------------------------------------*/
void IO::send_ENQ()
{
    CURRENT_STATE = REQUEST_LINE;
    if(IDLE_EOT_send_timer->isActive()){
       IDLE_EOT_send_timer->stop();
    }
    if(IDLE_EOT_received_timer->isActive()){
       IDLE_EOT_received_timer->stop();
    }
    emit write_to_port_signal(ENQ_FRAME);

}

/*------------------------------------------------------------------------------------------------------------------
--FUNCTION:     send_ENQ_after_backoff
--
--DATE:         Nov 22, 2018    Simon Chen - created
--
--DESIGNER:     Simon Chen
--
--PROGRAMMER:   Simon Chen
--
--INTERFACE:    void send_ENQ_after_backoff(void)
--
--RETURNS:      void.
--
--NOTES:
--This function servers as a special send ENQ to prevent simutaneous ENQ requests.
----------------------------------------------------------------------------------------------------------------------*/
void IO::send_ENQ_after_backoff(){
    if(backingOff){
        emit write_to_port_signal(ENQ_FRAME);
        backingOff = false;
    }
}

/*------------------------------------------------------------------------------------------------------------------
--FUNCTION:     resend_frame
--
--DATE:         Nov 28, 2018    Simon Chen - created
--
--DESIGNER:     Simon Chen
--
--PROGRAMMER:   Simon Chen
--
--INTERFACE:    void resend_frame(void)
--
--RETURNS:      void.
--
--NOTES:
--This function checks if resend attempts are within Max range before making resending frame.
----------------------------------------------------------------------------------------------------------------------*/
void IO::resend_frame(){
    if(resend_counts < MAX_RESENDS){

        resend_DATA_FRAME();

    } else {
        CURRENT_STATE = IDLE;
        resend_counts = 0;
        retransmission_Timer->stop();
        IDLE_EOT_send_timer->start(EOT_TIMEOUT);
        IDLE_EOT_received_timer->start(30000);
    }
}

/*------------------------------------------------------------------------------------------------------------------
--FUNCTION:     IDLE_send_EOT
--
--DATE:         Dec. 7, 2018    Daniel Shin - created
--
--DESIGNER:     Daniel Shin
--
--PROGRAMMER:   Daniel Shin
--
--INTERFACE:    void IDLE_send_EOT(void)
--
--RETURNS:      void.
--
--NOTES:
--This function deals with EOT sending in Idle state.
----------------------------------------------------------------------------------------------------------------------*/
void IO::IDLE_send_EOT()
{

    if(CURRENT_STATE == IDLE){
        //qDebug() << "OOOOOOOOOOOOOOOOOOO: sent EOT";
        IDLE_EOT_send_timer->start(EOT_TIMEOUT);
    }
    emit write_to_port_signal(EOT_FRAME);
    return;
}

/*------------------------------------------------------------------------------------------------------------------
--FUNCTION:     IDLE_EOT_not_received
--
--DATE:         Dec. 7, 2018    Daniel Shin - created
--
--DESIGNER:     Daniel Shin
--
--PROGRAMMER:   Daniel Shin
--
--INTERFACE:    void IDLE_EOT_not_received(void)
--
--RETURNS:      void.
--
--NOTES:
--This function deals with disconnection in idle state.
----------------------------------------------------------------------------------------------------------------------*/
void IO::IDLE_EOT_not_received()
{
    if(CURRENT_STATE == IDLE && EOT_received == false){
        qDebug() << "OOOOOOOOOOOOOOOOOOO: EOT not received after 30s. DISCONNECT";
        terminate_program();
    }else{

        IDLE_EOT_received_timer->start(30000);
        EOT_received = false;
    }
    return;
}

/*------------------------------------------------------------------------------------------------------------------
--FUNCTION:     terminate_program
--
--DATE:         Dec. 7, 2018    Daniel Shin - created
--
--DESIGNER:     Daniel Shin
--
--PROGRAMMER:   Daniel Shin
--
--INTERFACE:    void terminate_program(void)
--
--RETURNS:      void.
--
--NOTES:
--Helper function to clean up before exiting thread back to main.
----------------------------------------------------------------------------------------------------------------------*/
void IO::terminate_program()
{
    disconnect(IDLE_EOT_send_timer, &QTimer::timeout, this, &IO::IDLE_send_EOT);
    disconnect(IDLE_EOT_received_timer, &QTimer::timeout, this, &IO::IDLE_EOT_not_received);

    serial_port->flush();
    serial_port->clear();
    serial_port->close();
    QThread::currentThread()->disconnect();
}


/*------------------------------------------------------------------------------------------------------------------
--FUNCTION:     send_ACK
--
--DATE:         Nov. 22, 2018   Daniel Shin- Skeleton
--
--REVISIONS:
--              Dec.  7, 2018   Simon Chen - Added receiver timer
--              Dec. 11, 2018   Allan Hsu  - Added statistics
--
--DESIGNER:     Daniel Shin
--
--PROGRAMMER:   Daniel Shin, Allan Hsu, Simon Chen
--
--INTERFACE:    void send_ACK(void)
--
--RETURNS:      void.
--
--NOTES:
--This function trigger send ACK event and ACK statistic count event
----------------------------------------------------------------------------------------------------------------------*/
void IO::send_ACK()
{
    ++NUM_ACK;
    emit ack_sent_signal_statistic();

    CURRENT_STATE = RECEIVE_FRAME;
    data_frame_receive_Timer->start(RECEIVE_TIMEOUT);
    emit write_to_port_signal(ACK_FRAME);
}

/*------------------------------------------------------------------------------------------------------------------
--FUNCTION:     send_NAK
--
--DATE:         Nov. 22, 2018   Daniel Shin- Skeleton
--
--REVISIONS:
--              Dec.  7, 2018   Simon Chen - Added receiver timer
--              Dec. 11, 2018   Allan Hsu  - Added statistics
--
--DESIGNER:     Daniel Shin
--
--PROGRAMMER:   Daniel Shin, Allan Hsu, Simon Chen
--
--INTERFACE:    void send_NAK(void)
--
--RETURNS:      void.
--
--NOTES:
--This function trigger send NAK event and NAK statistic count event
----------------------------------------------------------------------------------------------------------------------*/
void IO::send_NAK()
{
    NUM_NAK++;
    emit nak_sent_signal_statistic();
    CURRENT_STATE = RESEND_FRAME;
    data_frame_receive_Timer->start(RECEIVE_TIMEOUT);
    emit write_to_port_signal(NAK_FRAME);
}

/*------------------------------------------------------------------------------------------------------------------
--FUNCTION:     send_DATA_FRAME
--
--DATE:         Nov. 28, 2018   Daniel Shin - First draft
--
--REVISIONS:
--              Dec.  7, 2018   Simon Chen - Added retransmission_Timer
--              Dec. 11, 2018   Simon Chen - Added statistics
--
--DESIGNER:     Daniel Shin
--
--PROGRAMMER:   Daniel Shin, Simon Chen
--
--INTERFACE:    void send_DATA_FRAME(void)
--
--RETURNS:      void.
--
--NOTES:
--This function class calls make_frame to create a frame from file read and
--triggers write to port event signal.
----------------------------------------------------------------------------------------------------------------------*/
void IO::send_DATA_FRAME()
{

    if(sendFrameCount < 10){
        QByteArray temp;
        temp = make_frame(file_handler->get_next());
        if(temp.at(1) != EOT){
            retransmission_Timer->start(TRANSMISSION_TIMEOUT);
        }

        emit write_to_port_signal(temp);
    } else {
        CURRENT_STATE = IDLE;
        sendFrameCount = 0;
        send_EOT();
        send_ENQ();

    }


}


/*------------------------------------------------------------------------------------------------------------------
--FUNCTION:     resend_DATA_FRAME
--
--DATE:         Dec.  5, 2018   Simon Chen - First draft
--
--REVISIONS:
--              Dec. 11, 2018   Allan Hsu - Added statistics
--
--DESIGNER:     Simon Chen
--
--PROGRAMMER:   Simon Chen
--
--INTERFACE:    void resend_DATA_FRAME(void)
--
--RETURNS:      void.
--
--NOTES:
--This function resends specifically checks for resend attempts
-- and triggers write to port event signal to resend data frames missed.
----------------------------------------------------------------------------------------------------------------------*/
void IO::resend_DATA_FRAME()
{
    if(resend_counts > 3){
        CURRENT_STATE = IDLE;
    } else {
        resend_counts ++;
        qDebug()<<"resend count"<<resend_counts;
        retransmission_Timer->start(TRANSMISSION_TIMEOUT);
        NUM_FRAME_SENT++;
        emit write_to_port_signal(make_frame(file_handler->get_prev()));
    }

}

/*------------------------------------------------------------------------------------------------------------------
--FUNCTION:     receive_timeout
--
--DATE:         Dec.  7, 2018   Simon Chen - First draft
--
--DESIGNER:     Simon Chen
--
--PROGRAMMER:   Simon Chen
--
--INTERFACE:    void receive_timeout(void)
--
--RETURNS:      void.
--
--NOTES:
--This function is called when an time out event happens in receive state.
----------------------------------------------------------------------------------------------------------------------*/
void IO::receive_timeout(){

    CURRENT_STATE = IDLE;
    IDLE_EOT_send_timer->start(EOT_TIMEOUT);
    IDLE_EOT_received_timer->start(30000);
}

/*------------------------------------------------------------------------------------------------------------------
--FUNCTION:     make_frame
--
--DATE:         Nov. 22, 2018   Daniel Shin - First draft
--
--REVISIONS:    Dec. 2, 2018    Daniel Shin - added idle timer
                Dec. 11, 2018   Allan Hsu   - fixed CRC usage
--
--DESIGNER:     Daniel Shin
--
--PROGRAMMER:   Daniel Shin
--
--INTERFACE:    QByteArray make_frame(const QByteArray &data)
--                      const QByteArray &data: makes a frame given data
--RETURNS:      QByteArray
--
--NOTES:
--This function is called when an time out event happens in receive state.
----------------------------------------------------------------------------------------------------------------------*/
QByteArray IO::make_frame(const QByteArray &data)
{

    if(data.isEmpty()){
        CURRENT_STATE = IDLE;
        IDLE_EOT_send_timer->start(EOT_TIMEOUT);
        IDLE_EOT_received_timer->start(30000);

        return EOT_FRAME;

    } else {

        QByteArray padding = QByteArray(DATA_LENGTH - data.size(), 0x14);

        //TODO: alternate DC1
        QByteArray frame;
        if (dcFlip == false) {
            frame = SYN_FRAME + DC1_FRAME + data + padding;

            qDebug()<<"sent dc1";
        } else {
            frame = SYN_FRAME + DC2_FRAME + data + padding;
            qDebug()<<"sent dc2";
        }
        uint32_t crc = CRC::Calculate(frame, 1023, CRC::CRC_32());
        frame.push_back(crc);
        return frame;
    }

}

/*------------------------------------------------------------------------------------------------------------------
--FUNCTION:     read_from_port
--
--DATE:         Nov. 28, 2018   Daniel Shin - First draft
--
--REVISIONS:    Dec. 11, 2018   Simon Chen  - added sync detection
--
--DESIGNER:     Daniel Shin
--
--PROGRAMMER:   Daniel Shin, Simon Chen
--
--INTERFACE:    void read_from_port(void)
--
--RETURNS:      void
--
--NOTES:
--This function is called when a ready to read event is triggered by serial port class,
--it reads data from port, checks for sync bit and triggers a data received signal event.
----------------------------------------------------------------------------------------------------------------------*/
void IO::read_from_port()
{
    int counter = 0;
    master_buffer.clear();
    master_buffer += serial_port->readAll();
    if(synDetected == false){
        while(synDetected == false && counter < master_buffer.size()){

            if(master_buffer[counter] == (char)SYN){
                synDetected = true;
                frame += master_buffer.mid(counter, master_buffer.size() - counter);
                qDebug()<<"entered control frame";
            } else {
                counter++;
            }
        }

    } else {
        frame += master_buffer;
    }

    emit data_received_signal(frame);

}


/*------------------------------------------------------------------------------------------------------------------
--FUNCTION:     handle_control_buffer
--
--DATE:         Nov. 22, 2018   Daniel Shin - Skeleton
--
--REVISIONS:    Dec.  2, 2018   Jenny Ly    - implemented state checking
--
--DESIGNER:     Daniel Shin
--
--PROGRAMMER:   Daniel Shin, Jenny Ly
--
--INTERFACE:    void handle_control_buffer(void)
--
--RETURNS:      void
--
--NOTES:
--This function checks control buffer read from control frame and calls corresponding
--message received functions.
----------------------------------------------------------------------------------------------------------------------*/
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

/*------------------------------------------------------------------------------------------------------------------
--FUNCTION:     received_ENQ
--
--DATE:         Nov. 22, 2018   Daniel Shin - skeleton
--
--REVISIONS:    Nov. 28, 2018   Daniel Shin - added idle send&receive timer
--              Dec.  2, 2018   Jenny Ly    - implemented state checking
--              Dec.  7, 2018   Simon Chen  - added backed off timer for situations
--
--DESIGNER:     Daniel Shin
--
--PROGRAMMER:   Daniel Shin, Jenny Ly, Simon Chen
--
--INTERFACE:    void received_ENQ(void)
--
--RETURNS:      void
--
--NOTES:
--This function responds to receiving ENQ during different state.
----------------------------------------------------------------------------------------------------------------------*/
void IO::received_ENQ(){
    switch(CURRENT_STATE){
    case IDLE:
        IDLE_EOT_send_timer->stop();
        IDLE_EOT_received_timer->stop();
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

/*------------------------------------------------------------------------------------------------------------------
--FUNCTION:     received_EOT
--
--DATE:         Nov. 22, 2018   Daniel Shin - skeleton
--
--REVISIONS:    Nov. 28, 2018   Daniel Shin - added idle send&receive timer
--              Dec.  2, 2018   Jenny Ly    - implemented state checking
--              Dec. 11, 2018   Simon Checn - added file saves
--DESIGNER:     Daniel Shin
--
--PROGRAMMER:   Daniel Shin, Jenny Ly
--
--INTERFACE:    void received_EOT(void)
--
--RETURNS:      void
--
--NOTES:
--This function responds to receiving EOT during different state.
----------------------------------------------------------------------------------------------------------------------*/
void IO::received_EOT(){

    switch(CURRENT_STATE){
    case IDLE:
        EOT_received = true;
        //IDLE_send_EOT();
        break;
    case REQUEST_LINE:
        CURRENT_STATE = SEND_STATE;
       // send_ACK();
        break;
    case SEND_STATE:
        CURRENT_STATE = IDLE;
        //retransmission_timeout->stop();
        break;
    case RESEND_FRAME:
        break;
    case RECEIVE_FRAME:
        CURRENT_STATE = IDLE;
        IDLE_EOT_send_timer->start(EOT_TIMEOUT);
        IDLE_EOT_received_timer->start(30000);
        file_handler->save_file(fileData);
        qDebug() <<"return to idle from receive xxxxxxxxxxxxxxxxxxxxxx";
        break;
    default:
        break;
    }
}

/*------------------------------------------------------------------------------------------------------------------
--FUNCTION:     received_NAK
--
--DATE:         Nov. 22, 2018   Daniel Shin - skeleton
--
--REVISIONS:    Dec.  2, 2018   Jenny Ly    - implemented state checking
--              Dec.  7, 2018   Simon Chen  - retransmission_Timer
--
--DESIGNER:     Daniel Shin
--
--PROGRAMMER:   Daniel Shin, Jenny Ly, Simon Chen
--
--INTERFACE:    void received_NAK(void)
--
--RETURNS:      void
--
--NOTES:
--This function responds to receiving NAK during different state.
----------------------------------------------------------------------------------------------------------------------*/
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

/*------------------------------------------------------------------------------------------------------------------
--FUNCTION:     received_ACK
--
--DATE:         Nov. 22, 2018   Daniel Shin - skeleton
--
--REVISIONS:    Dec.  2, 2018   Jenny Ly    - implemented state checking
--              Dec.  7, 2018   Simon Chen  - retransmission_Timer
--
--DESIGNER:     Daniel Shin
--
--PROGRAMMER:   Daniel Shin, Jenny Ly
--
--INTERFACE:    void received_ACK(void)
--
--RETURNS:      void
--
--NOTES:
--This function responds to receiving NAK during different state.
----------------------------------------------------------------------------------------------------------------------*/
void IO::received_ACK(){

qDebug()<<"CURRENT STATE "<<CURRENT_STATE;
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
            dcFlip = !dcFlip;
            sendFrameCount++;
            emit frame_sent_signal_statistic();
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

/*------------------------------------------------------------------------------------------------------------------
--FUNCTION:     process_frames
--
--DATE:         Nov. 22, 2018   Daniel Shin - skeleton
--
--REVISIONS:    Dec. 11, 2018   Allan Hsu   - modified data_frame_receive_Timer
--              Dec. 11, 2018   Simon Chen  - added CRC check
--
--DESIGNER:     Daniel Shin
--
--PROGRAMMER:   Daniel Shin, Simon Chen, Allan Hsu
--
--INTERFACE:    void process_frames(QString data)
--                      QString data: data as frames to be processed
--RETURNS:      void
--
--NOTES:
--This function checks if the data is a control frame or data frame and calls
--helper functions to do the work.
----------------------------------------------------------------------------------------------------------------------*/
void IO::process_frames(QString data){

    data_buffer = "";
    control_buffer.clear();


    if(data[0] == (char)SYN
            && (data[2] == (char)DC1 || data[2] == (char)DC2)){
        qDebug()<<"entered process frame if " <<frame.size();
        synDetected = false;
        control_buffer = frame.left(3);//data.toUtf8();
        qDebug() << "it's a control frame!";
        qDebug() << control_buffer;
        handle_control_buffer();
        frame = frame.right(frame.size() - 3);

        if(!frame.isEmpty()){
            control_buffer = frame.left(3);
            handle_control_buffer();
            frame.clear();
        }

    } else {

      // qDebug()<<"entered process frame else " <<frame.size();
       if(frame.size() == 1024){
           if(data_frame_receive_Timer->isActive()){
              data_frame_receive_Timer->stop();
           }
           synDetected = false;
           if (dcFlipReceive == false && frame.at(1) == DC1) {//dc1
              data_buffer = data;
              qDebug() << "it's a data frame with DC1!";
              qDebug() << data;
              dcFlipReceive = !dcFlipReceive;

              //check crc
               uint32_t crcTemp = CRC::Calculate(frame.left(1023), 1023, CRC::CRC_32());
               QByteArray temp2;
               QByteArray temp3;
               temp3 += crcTemp;
               temp2 += frame[1023];
               if(temp2.toHex() == temp3.toHex()){
                   int paddingCounter = 1;
                   while(frame[paddingCounter]!= (char)0x14 && paddingCounter < frame.size()){
                       paddingCounter++;
                   }
                   fileData += frame.mid(2,paddingCounter-3);
                   data_buffer = data.mid(2,paddingCounter-3);
                   emit(ready_to_print_signal());
                   paddingCounter = 1;




                   send_ACK();
               } else {
                   send_NAK();
               }

              frame.clear();
           } else if (dcFlipReceive == true && frame.at(1) == DC2) {//dc1
               data_buffer = data;
               qDebug() << "it's a data frame with DC2!";
               qDebug() << data;
               dcFlipReceive = !dcFlipReceive;
               //check crc
                uint32_t crcTemp = CRC::Calculate(frame.left(1023), 1023, CRC::CRC_32());
                QByteArray temp2;
                QByteArray temp3;
                temp3 += crcTemp;
                temp2 += frame[1023];
                if(temp2.toHex() == temp3.toHex()){
                    int paddingCounter = 1;
                    while(frame[paddingCounter]!= (char)0x14 && paddingCounter < frame.size()){
                        paddingCounter++;
                    }
                    fileData += frame.mid(2,paddingCounter-3);

                    data_buffer = data.mid(2,paddingCounter-3);

                    emit(ready_to_print_signal());
                    paddingCounter = 1;
                    send_ACK();
                } else {
                    send_NAK();
                }
               frame.clear();
            }
        }
    }

}

//TODO:
//clean up
//comment
//user manual
//test documents
//design doc

