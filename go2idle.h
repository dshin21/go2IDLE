#ifndef GO2IDLE_H
#define GO2IDLE_H

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

public slots:
    void select_file();
};

#endif // GO2IDLE_H
