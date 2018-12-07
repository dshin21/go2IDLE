#ifndef CONSTANTS_H
#define CONSTANTS_H
#include <QByteArray>

#define IDLE 1 // SENT EOT
#define REQUEST_LINE 2 // SENT ENQ
#define SEND_STATE 3 // SENT data FRAME
#define WAIT_RESPONSE 4 // SENT control FRAME
#define RESEND_FRAME 5 // sent NAK
#define RECEIVE_FRAME 6 // sent ack
#define TRANSMISSION_TIMEOUT 1

static int CURRENT_STATE = 1;
static int CURRENT_FRAME_ID = 1;
static int LAST_FRAME_ID = 2;

const int DATA_FRAME_LENGTH = 1024;
const int DATA_LENGTH = 1021;
const int CONTROL_FRAME_LENGTH = 3;
const int MAX_RESENDS = 3;
const int FRAME1 = 1;
const int FRAME2 = 2;

const int SYN = 0x16;
const QByteArray SYN_FRAME = QByteArray(1, SYN);

const int DC1 = 0x11;
const QByteArray DC1_FRAME = QByteArray(1, DC1);

const int DC2 = 0x12;
const QByteArray DC2_FRAME = QByteArray(1, DC2);

const int EOT = 0x04;
const QByteArray EOT_FRAME = SYN_FRAME + QByteArray(1, EOT) + DC1_FRAME;

const int ENQ = 0x05;
const QByteArray ENQ_FRAME = SYN_FRAME + QByteArray(1, ENQ) + DC1_FRAME;

const int ACK = 0x06;
const QByteArray ACK_FRAME = SYN_FRAME + QByteArray(1, ACK) + DC1_FRAME;

const int NAK = 0x15;
const QByteArray NAK_FRAME = SYN_FRAME + QByteArray(1, NAK)  + DC1_FRAME;

static int NUM_ACK = 0;
static int NUM_NAK = 0;
static int BE_RATE = 0;
static int NUM_FRAME_SENT = 0;


#endif // CONSTANTS_H
