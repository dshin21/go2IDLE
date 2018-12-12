#ifndef GO2IDLE_H
#define GO2IDLE_H

#include "IO.h"

#include <QMainWindow>
#include <QFileDialog>

namespace Ui {
class go2IDLE;
}

class go2IDLE : public QMainWindow
{
    Q_OBJECT

public:
    explicit go2IDLE(QWidget *parent = nullptr);
    ~go2IDLE();

private:
    Ui::go2IDLE *ui;
    IO* io_thread;
    int ackCount = 0;
    int nakCount = 0;
    int frameCount = 0;
    int beRateCount = 0;

signals:
//    void IDLE_send_EOT_signal();

public slots:
    void display_data();
    void update_ack();
    void update_nak();
    void update_frame();
    void update_beRate();
};

#endif // GO2IDLE_H
