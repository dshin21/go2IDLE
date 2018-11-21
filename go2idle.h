#ifndef GO2IDLE_H
#define GO2IDLE_H

#include <QMainWindow>

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
};

#endif // GO2IDLE_H
