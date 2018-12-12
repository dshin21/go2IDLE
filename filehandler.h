#ifndef FILEHANDLER_H
#define FILEHANDLER_H

#include<QByteArray>
#include<QFileDialog>
#include<QWidget>
#include<fstream>
#include<string>
#include <QIODevice>

using namespace std;

class FileHandler : public QWidget
{
    Q_OBJECT
private:
    int characterCount = 0;
    bool endOfFile = false;
public:
    ifstream  *if_stream;
    char buffer[512];
    string fileName;
    QByteArray data_buffer;

    FileHandler(QObject *parent = nullptr);
    //    ~FileHandler();
    QByteArray get_next();
    QByteArray get_prev();
    void save_file(QByteArray data);
public slots:
    void select_file();

};

#endif // FILEHANDLER_H
