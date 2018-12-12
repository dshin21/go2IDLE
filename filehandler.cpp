/*------------------------------------------------------------------------------------------------------------------
--SOURCE FILE:  FileHandler.cpp-   File handler application to write and save a file
--
--PROGRAM:      Radio Modem Protocol Driver
--
--FUNCTIONS:
--              QByteArray get_next();
--              QByteArray get_prev();
--              void save_file(QByteArray data);
--
--DATE:         Nov 22, 2018  Daniel Shin - first draft
--
--REVISIONS:    Nov 28, 2018  Daniel Shin - modified file reading
--              Dec. 7, 2018  Simon Chen  - modified get_next()
--              Dec.11, 2018  Simon Chen  - modified select_file()
--
--DESIGNER:     Daniel Shin
--
--PROGRAMMER:   Daniel Shin, Simon Chen
--
--NOTES:
--Helper class to allow write to a file for data transmitting purpose.
----------------------------------------------------------------------------------------------------------------------*/
#include "filehandler.h"

FileHandler::FileHandler(QObject *parent)
    : if_stream(new ifstream)
{

}

/*------------------------------------------------------------------------------------------------------------------
--FUNCTION:     get_next
--
--DATE:         Nov. 22, 2018   Daniel Shin - first draft
--
--REVISIONS:    Dec. 7, 2018  Simon Chen  - fixed file reading bug
--
--DESIGNER:     Daniel Shin
--
--PROGRAMMER:   Daniel Shin, Simon Chen
--
--INTERFACE:    QByteArray get_next(void)
--
--RETURNS:      QByteArray
--
--NOTES:
--This function gets the next fixed length of data from file for making frames to transmit.
----------------------------------------------------------------------------------------------------------------------*/
QByteArray FileHandler::get_next()
{
    char temp;
    data_buffer.clear();
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

/*------------------------------------------------------------------------------------------------------------------
--FUNCTION:     get_prev
--
--DATE:         Nov. 22, 2018   Daniel Shin - first draft
--
--REVISIONS:    Dec. 11, 2018  Simon Chen  - fixed variable
--
--DESIGNER:     Daniel Shin
--
--PROGRAMMER:   Daniel Shin, Simon Chen
--
--INTERFACE:    QByteArray get_prev(void)
--
--RETURNS:      QByteArray
--
--NOTES:
--This function gets previous fixed length of data from file for making frames to retransmit.
----------------------------------------------------------------------------------------------------------------------*/
QByteArray FileHandler::get_prev()
{
    return QByteArray(data_buffer);
}

/*------------------------------------------------------------------------------------------------------------------
--FUNCTION:     select_file
--
--DATE:         Nov. 22, 2018   Daniel Shin - first draft
--
--DESIGNER:     Daniel Shin
--
--PROGRAMMER:   Daniel Shin
--
--INTERFACE:    void select_file(void)
--
--RETURNS:      void
--
--NOTES:
--This function allows user to select files to transmit.
----------------------------------------------------------------------------------------------------------------------*/
void FileHandler::select_file(){
    fileName = QFileDialog::getOpenFileName(
                this,
                tr("Select a File to Send"),
                "",
                tr("Text File (*.txt)")
                ).toStdString();
   if_stream->open(fileName, fstream::in | fstream::binary);
}

/*------------------------------------------------------------------------------------------------------------------
--FUNCTION:     save_file
--
--DATE:         Dec. 11, 2018   Simon Chen  - created
--
--DESIGNER:     Simon Chen
--
--PROGRAMMER:   Simon Chen
--
--INTERFACE:    void save_file(QByteArray data)
--
--RETURNS:      void
--
--NOTES:
--This function allows receiver to save data transmitted into a file.
----------------------------------------------------------------------------------------------------------------------*/
void FileHandler::save_file(QByteArray data){
    QFile file("some_name.txt");
    file.open(QIODevice::WriteOnly);
    file.write(data);
    file.close();
}

