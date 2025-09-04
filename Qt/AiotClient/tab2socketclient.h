#ifndef TAB2SOCKETCLIENT_H
#define TAB2SOCKETCLIENT_H

#include <QWidget>
#include <QDebug>
#include <QTime>
#include <QLineEdit>
#include <QDebug>

#include "keyboard.h"
#include "sockclient.h"

namespace Ui {
class Tab2socketclient;
}

class Tab2socketclient : public QWidget
{
    Q_OBJECT
public:
    explicit Tab2socketclient(QWidget *parent = nullptr);
    ~Tab2socketclient();
    SockClient * getpSockClient();

private slots:
    void on_pPBserverConnect_toggled(bool checked);
    void updateRecvDataSlot(QString);

    void on_pPBrecvDataClear_clicked();

    void on_pPBSend_clicked();
    void socketSendToLinux(int);

    void on_pLErecvid_selectionChanged();

    void on_pLEsendData_selectionChanged();

signals:
    void ledWriteSig(int);
    void tab3RecvDataSig(QString);
    void Tab4RecvDataSig(QString);
    void Tab5RecvDataSig(QString);

private:
    Ui::Tab2socketclient *ui;
    SockClient *pSockClient; // 소켓 클라이언트 포인터 변수
    Keyboard *pKeyboard;
};

#endif // TAB2SOCKETCLIENT_H
