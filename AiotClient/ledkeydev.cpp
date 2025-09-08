#include "ledkeydev.h"

LedKeyDev::LedKeyDev(QWidget *parent)
    : QWidget{parent}
{
    int ledKeyFd;
    pQFile = new QFile(DEVFILENAME);
    if(!pQFile->open(QFile::ReadWrite | QFile::Unbuffered))
    {
        QMessageBox::information(this, "open", "open faile : "+DEVFILENAME);
    }
    ledKeyFd = pQFile->handle();
    pQSocketNotifier = new QSocketNotifier(ledKeyFd, QSocketNotifier::Read,this);
    connect(pQSocketNotifier, SIGNAL(activated(int)),this, SLOT(readKeyDataSlot(int)));
    writeLedDataSlot(0);
}

void LedKeyDev::readKeyDataSlot(int)
{
    char keyNo = 0;
    int ret = pQFile->read(&keyNo,sizeof(keyNo));
    if(ret > 0)
        emit updateKeyDataSig(int(keyNo));
    // qDebug() << int(keyNo);
}
void LedKeyDev::writeLedDataSlot(int ledValue)
{
    char ledNo = (char)ledValue;
    pQFile->write(&ledNo, sizeof(ledNo));
    qDebug() << (unsigned char)(ledNo);
}
