#ifndef FILEHANDLER_H
#define FILEHANDLER_H

#include<QByteArray>
#include<QFileDialog>
#include<QWidget>
#include<fstream>
#include<string>

using namespace std;

class FileHandler : public QWidget
{
    Q_OBJECT
private:
    int characterCount = 0;
    bool endOfFile = false;
    QByteArray previousFrame;
public:
    ifstream  *if_stream;
    char buffer[512];
    string fileName;

    FileHandler(QObject *parent = nullptr);
    //    ~FileHandler();
    QByteArray get_next();
    QByteArray get_prev();
public slots:
    void select_file();

};

#endif // FILEHANDLER_H
