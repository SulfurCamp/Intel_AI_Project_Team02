#ifndef SOCKCLIENT_H
#define SOCKCLIENT_H
// sockclient.h 는 아무것도 건들지 말 것

#include <QWidget>
#include <QTcpSocket>
#include <QHostAddress>
#include <QInputDialog>
#include <QMessageBox>
#include <QDebug>

#define BLOCK_SIZE 1024

class SockClient : public QWidget
{
    Q_OBJECT
    QTcpSocket *pQTcpSocket;
    QString SERVERIP = "192.168.0.172"; // 서버 주소
    int SERVERPORT = 5000;
    QString LOGID = "QTCLIENT";
    QString LOGPW = "PASSWD";
public:
    explicit SockClient(QWidget *parent = nullptr);
    ~SockClient();

signals:
    void socketRevcDataSig(QString);
    void connectedSig();
    void disconnectedSig();
    void errorSig(QString);

private slots:
    void socketReadDataSlot();
    void socketErrorSlot();
    void socketConnectServerSlot();

public slots:
    void connectToServerSlot(bool &);
    void socketClosedServerSlot();
    void socketWriteDataSlot(QString);
};

#endif // SOCKCLIENT_H
