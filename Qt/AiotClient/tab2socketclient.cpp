#include "tab2socketclient.h"
#include "ui_tab2socketclient.h"

Tab2socketclient::Tab2socketclient(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Tab2socketclient)
{
    ui->setupUi(this);

    ui->pPBSend->setEnabled(false);
    pSockClient = new SockClient(this); // 소켓 객체 생성

    pKeyboard = new Keyboard();

    connect(pSockClient, SIGNAL(socketRevcDataSig(QString)), this, SLOT(updateRecvDataSlot(QString)));
    // on_pPBserverConnect_toggled(true); // 실행하면 서버 연결창이 나옴
}

Tab2socketclient::~Tab2socketclient()
{
    delete ui;
}

void Tab2socketclient::on_pPBserverConnect_toggled(bool checked)
{
    bool bFlag;
    // qDebug() << "on_pushButton_2_toggled()";
    if(checked)
    {
        pSockClient->connectToServerSlot(bFlag);
        if(bFlag)
        {
            ui->pPBserverConnect->setText("서버 해제");
            ui->pPBSend->setEnabled(true); // 서버 연결하면 버튼 활성화
        }
    }
    else
    {
        pSockClient->socketClosedServerSlot();
        ui->pPBserverConnect->setText("서버 연결");
        ui->pPBSend->setEnabled(false); // 서버 해제되면 버튼 비활성화
    }
}

void Tab2socketclient::updateRecvDataSlot(QString strRecvData)
{
    strRecvData.chop(1); // 끝 문자 한개 즉'\n' 제거
    QTime time = QTime::currentTime(); // 현재 시간
    QString strTime = time.toString(); // 문자열로 변환
    strTime = strTime + " " + strRecvData;
    ui->pTErecvdata->append(strTime);

    // [KMS_QT]@LED@0xff -> @KMS_QT@LED@0xff
    strRecvData.replace("[", "@");
    strRecvData.replace("]", "@");

    QStringList strList = strRecvData.split("@"); // 문자열로 분리 @KMS_QT@LED@0xff -> ["","KMS_QT", "LED", "0xff"]

    if(strList[2].indexOf("LED") == 0) // 특정 문자열이 처음 나타나는 위치(인덱스)를 찾고, 없으면 -1을 반환한다.
    {
        bool bFlag;
        int ledNo = strList[3].toInt(&bFlag, 16);
        if(bFlag)
            emit ledWriteSig(ledNo);
    }
    else if(strList[2].indexOf("LAMP") == 0 || strList[2].indexOf("PLUG") == 0)
    {
        // qDebug() << strRecvData;
        emit tab3RecvDataSig(strRecvData);
    }
    else if(strList[2].indexOf("SENSOR") == 0)
    {
        emit Tab4RecvDataSig(strRecvData);
        emit Tab5RecvDataSig(strRecvData);
    }
}

void Tab2socketclient::on_pPBrecvDataClear_clicked()
{
    ui->pTErecvdata->clear();
}


void Tab2socketclient::on_pPBSend_clicked()
{
    QString strRecvId = ui->pLErecvid->text(); // 문자열 가져오기
    QString strSendData = ui->pLEsendData->text();
    if(strRecvId.isEmpty()) // 누구한테 보낼지 안정했으면
    {
        strSendData = "[ALLMSG]" + strSendData; // 모든 클라이언트에게 전송
    }
    else
    {
        strSendData = "[" + strRecvId + "]" + strSendData;
    }
    pSockClient->socketWriteDataSlot(strSendData);
    ui->pLEsendData->clear();
}

void Tab2socketclient::socketSendToLinux(int keyNo)
{
    pSockClient->socketWriteDataSlot("[KMS_LIN]KEY@" + QString::number(keyNo));
}

SockClient * Tab2socketclient::getpSockClient()
{
    return pSockClient;
}

void Tab2socketclient::on_pLErecvid_selectionChanged()
{
    QLineEdit *pQLineEdit = (QLineEdit *)sender();
    pKeyboard->setLineEdit(pQLineEdit);
    pKeyboard->show();
}


void Tab2socketclient::on_pLEsendData_selectionChanged()
{
    QLineEdit *pQLineEdit = (QLineEdit *)sender();
    pKeyboard->setLineEdit(pQLineEdit);
    pKeyboard->show();
}

