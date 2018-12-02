#ifndef CONSTANTS_H
#define CONSTANTS_H
#include <QByteArray>

const int DATA_FRAME_LENGTH = 1024;
const int DATA_LENGTH = 1021;
const int CONTROL_FRAME_LENGTH = 3;

const int SYN = 0x16;
const QByteArray SYN_FRAME = QByteArray(1, SYN);

const int DC1 = 0x11;
const QByteArray DC1_FRAME = QByteArray(1, DC1);

const int DC2 = 0x12;
const QByteArray DC2_FRAME = QByteArray(1, DC2);

const int EOT = 0x04;
const QByteArray EOT_FRAME = SYN_FRAME + QByteArray(1, EOT);

const int ENQ = 0x05;
const QByteArray ENQ_FRAME = SYN_FRAME + QByteArray(1, ENQ);

const int ACK = 0x06;
const QByteArray ACK_FRAME = SYN_FRAME + QByteArray(1, ACK);

const int NAK = 0x15;
const QByteArray NAK_FRAME = SYN_FRAME + QByteArray(1, NAK);

#endif // CONSTANTS_H
