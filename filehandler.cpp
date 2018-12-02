#include "filehandler.h"

FileHandler::FileHandler(QObject *parent)
    : if_stream(new ifstream)
{

}

QByteArray FileHandler::get_next()
{
    if_stream->get(buffer,512,EOF);
    return QByteArray(buffer);
}

QByteArray FileHandler::get_prev()
{
    return QByteArray(buffer);
}

void FileHandler::select_file(){
    fileName = QFileDialog::getOpenFileName(
                this,
                tr("Select a File to Send"),
                "",
                tr("Text File (*.txt)")
                ).toStdString();
   if_stream->open(fileName, fstream::in | fstream::binary);
}
