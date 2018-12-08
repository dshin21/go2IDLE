#include "filehandler.h"

FileHandler::FileHandler(QObject *parent)
    : if_stream(new ifstream)
{

}

QByteArray FileHandler::get_next()
{
    char temp;
    QByteArray data_buffer;
    characterCount = 0;

    while(!if_stream->eof() && characterCount < 1021){
        temp = if_stream->get();
        data_buffer += temp;
        characterCount++;
    }

    if(endOfFile){
        endOfFile = false;
        return NULL;
    }else{
        if(if_stream ->eof()){
            endOfFile = true;
        }
        return data_buffer;
    }
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
