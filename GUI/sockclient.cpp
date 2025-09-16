// sockclient.cpp 는 아무것도 건들지 말 것.

#include "sockclient.h"

SockClient::SockClient(QWidget *parent)
    : QWidget{parent}
{
    pQTcpSocket = new QTcpSocket();

    // 서버에 성공적으로 연결되었을 때 호출될 슬롯을 연결
    connect(pQTcpSocket, SIGNAL(connected()), this, SLOT(socketConnectServerSlot()));

    // 서버와의 연결이 끊겼을 때 호출될 슬롯을 연결
    connect(pQTcpSocket, SIGNAL(disconnected()), this, SLOT(socketClosedServerSlot()));

    // 수신할 데이터가 도착했을 때 호출될 슬롯을 연결
    connect(pQTcpSocket, SIGNAL(readyRead()), this, SLOT(socketReadDataSlot()));

    // 소켓 오류가 발생했을 때 호출될 슬롯을 연결
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    connect(pQTcpSocket, SIGNAL(errorOccurred(QAbstractSocket::SocketError)), this, SLOT(socketErrorSlot()));
#else
    connect(pQTcpSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(socketErrorSlot()));
#endif
}

void SockClient::connectToServerSlot(bool &bFlag)
{
    pQTcpSocket->connectToHost(SERVERIP, SERVERPORT);
    bFlag = true;
}

void SockClient::socketReadDataSlot()
{
    QByteArray byteRecvData;
    QString strRecvData;
    if(pQTcpSocket->bytesAvailable() > BLOCK_SIZE)
        return;
    byteRecvData = pQTcpSocket->read(BLOCK_SIZE);
    strRecvData = QString::fromLocal8Bit(byteRecvData);
    emit socketRevcDataSig(strRecvData);
}

void SockClient::socketErrorSlot()
{
    QString strError = pQTcpSocket->errorString();
    emit errorSig(strError);
}
void SockClient::socketConnectServerSlot()
{
    QString strIdPw = "[" + LOGID + ":" + LOGPW + "]";
    // qDebug() << strIdPw;
    QByteArray byteIdPw = strIdPw.toLocal8Bit(); // QString(유니코드)를 현재 OS의 기본 인코딩(CP949, UTF-8 등)을 사용하는 배열로 변환
    pQTcpSocket->write(byteIdPw);
    emit connectedSig();
}

void SockClient::socketClosedServerSlot()
{
    emit disconnectedSig();
    pQTcpSocket->close(); // 서버 연결 닫기
}

void SockClient::socketWriteDataSlot(QString strData)
{
    strData = strData + "\n";
    QByteArray byteData = strData.toLocal8Bit(); // QString(유니코드)를 현재 OS의 기본 인코딩(CP949, UTF-8 등)을 사용하는 배열로 변환
    pQTcpSocket->write(byteData);
}

SockClient::~SockClient()
{

}

void SockClient::connectWithAddress(const QString& ip, quint16 port)
{
    pQTcpSocket->connectToHost(ip, port);
}
