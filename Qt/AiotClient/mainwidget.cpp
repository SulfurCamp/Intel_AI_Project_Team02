#include "mainwidget.h"
#include "ui_mainwidget.h"

MainWidget::MainWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::MainWidget)
{
    ui->setupUi(this);

    // tab1
    pTab1DevControl = new Tab1DevControl(ui->pTab1);
    ui->pTab1->setLayout(pTab1DevControl->layout());

    // tab2
    pTab2socketclient = new Tab2socketclient(ui->pTab2);
    ui->pTab2->setLayout(pTab2socketclient->layout());

    // tab3
    pTab3ControlPannel = new Tab3ControlPannel(ui->pTab3);
    ui->pTab3->setLayout(pTab3ControlPannel->layout());

    // tab4
    pTab4SensorChart = new Tab4SensorChart(ui->pTab4);
    ui->pTab4->setLayout(pTab4SensorChart->layout());

    // tab5
    pTab5sensordatabase = new Tab5sensordatabase(ui->pTab5);
    ui->pTab5->setLayout(pTab5sensordatabase->layout());

    // tab6
    pTab6WebCamera = new Tab6WebCamera(ui->pTab6);
    ui->pTab6->setLayout(pTab6WebCamera->layout());

    // tab7
    pTab7CamViewerThread = new Tab7CamViewerThread(ui->pTab7);
    ui->pTab7->setLayout(pTab7CamViewerThread->layout());

    ui->pTabWidget->setCurrentIndex(0); // 어떤 tab이 디폴트로 실행되게 할지

    // Tab2에서 다른 클라이언트가 [KMS_QT]@LED@0xff 등과 같이 명령어가 오면 Tab1에서 Dial에 객체 포인터를 리턴받아 값을 Dial(lcd와 led도 바뀜)에 값을 바꾼다.
    connect(pTab2socketclient, SIGNAL(ledWriteSig(int)), pTab1DevControl->getpDial(), SLOT(setValue(int)));

    // Tab1에서 키(버튼)을 누르면 Tab2에서 리눅스에게 메시지를 보낸다.
    connect(pTab1DevControl->getpLedKeyDev(), SIGNAL(updateKeyDataSig(int)), pTab2socketclient, SLOT(socketSendToLinux(int)));

    // Tab3에서 LAMP버튼을 누르면 Tab2에서 sockclient 객체의 포인터를 리턴하며 다른 클라이언트에게 메시지를 보낸다.
    connect(pTab3ControlPannel, SIGNAL(socketSendDataSig(QString)), pTab2socketclient->getpSockClient(), SLOT(socketWriteDataSlot(QString)));

    // Tab2에서 다른 클라이언트가 LAMP나 PLUG가 들어간 명령어를 보내면 Tab3에서 버튼의 상태 변경
    connect(pTab2socketclient, SIGNAL(tab3RecvDataSig(QString)), pTab3ControlPannel, SLOT(tab3RecvDataSlot(QString)));

    // Tab2에서 SENSOR값이 오면 Tab4에서 그래프에 그린다.
    connect(pTab2socketclient, SIGNAL(Tab4RecvDataSig(QString)), pTab4SensorChart, SLOT(Tab4RecvDataSlot(QString)));
    connect(pTab2socketclient, SIGNAL(Tab5RecvDataSig(QString)), pTab5sensordatabase, SLOT(Tab5RecvDataSlot(QString)));

    // Tab7에서 카메라로 다른 색상을 읽어오면 KMS_LIN에게 메시지를 보낸다.
    connect(pTab7CamViewerThread->getWebCamThread(), SIGNAL(socketSendDataSig(QString)), pTab2socketclient->getpSockClient(), SLOT(socketWriteDataSlot(QString)));

}

MainWidget::~MainWidget()
{
    delete ui;
}
