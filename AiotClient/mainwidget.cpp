#include "mainwidget.h"
#include "ui_mainwidget.h"
#include <QVBoxLayout>

MainWidget::MainWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::MainWidget)
{
    ui->setupUi(this);

    // ── Tab1: 데이터/서버 (Tab1Data 안에 터미널/조회/테이블/차트 모두 포함)
    pTab1Data = new Tab1Data(this);
    {
        // ui의 첫 번째 탭 컨테이너 위젯 이름이 예전엔 pTab1 이었음
        // (당신 UI 이름이 다르면 해당 객체명으로 바꿔줘)
        QWidget *container = ui->pTab1;  // <- mainwidget.ui 탭1의 objectName
        auto *lay = container->layout();
        if (!lay) {
            lay = new QVBoxLayout(container);
            lay->setContentsMargins(0,0,0,0);
            container->setLayout(lay);
        }
        lay->addWidget(pTab1Data);
    }

    // ── Tab2: 카메라
    pTab2CamStream = new Tab2CamStream(this);
    {
        QWidget *container = ui->pTab2; // Corrected from ui->Tab1_DB
        auto *lay = container->layout();
        if (!lay) {
            lay = new QVBoxLayout(container);
            lay->setContentsMargins(0,0,0,0);
            container->setLayout(lay);
        }
        lay->addWidget(pTab2CamStream);
    }

    // 디폴트 탭
    ui->pTabWidget->setCurrentIndex(0);
    // ── 예전 탭 간 시그널 연결은 모두 제거 (필요 없고 클래스도 없음)
}

MainWidget::~MainWidget()
{
    delete ui;
}
