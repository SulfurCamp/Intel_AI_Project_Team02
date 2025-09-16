#include "mainwidget.h"
#include "tab1data.h"
#include "tab3dgraph.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QStackedWidget>
#include <QLabel>
#include <QPushButton>
#include <QFont>
#include <QFileInfo>
#include <QUrl>

// Multimedia
#include <QMediaPlayer>
#include <QMediaPlaylist>
#include <QVideoWidget>

MainWidget::MainWidget(QWidget *parent)
    : QWidget(parent)
{
    setupUi();
}

MainWidget::~MainWidget()
{
    // 부모-자식 소멸 규칙으로 자동 해제
}

void MainWidget::setupUi()
{
    // Main Layout
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    stackedWidget = new QStackedWidget(this);

    // --- Menu Page (Index 0) ---
    menuPage = new QWidget();
    auto *menuLayout = new QVBoxLayout(menuPage);
    menuLayout->setContentsMargins(0,0,0,0);
    menuLayout->setSpacing(8);

    // Title
    titleLabel = new QLabel("Drone Detection Turret", menuPage);
    titleLabel->setAlignment(Qt::AlignCenter);
    QFont titleFont = titleLabel->font();
    titleFont.setPointSize(24);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);
    titleLabel->setMaximumHeight(52);

    // Background VIDEO (loop)
    bgVideoWidget   = new QVideoWidget(menuPage);
    bgVideoPlayer   = new QMediaPlayer(menuPage);
    bgVideoPlaylist = new QMediaPlaylist(menuPage);

    const QString videoPath = "/home/ubuntu/Intel_AI_Project_Team02/Qt/Intel_AI_Qt/Images/background.mp4";
    if (!QFileInfo::exists(videoPath)) {
        qWarning() << "[VIDEO] not found:" << videoPath;
    }
    bgVideoPlaylist->addMedia(QUrl::fromLocalFile(videoPath));
    bgVideoPlaylist->setPlaybackMode(QMediaPlaylist::Loop);
    bgVideoPlayer->setPlaylist(bgVideoPlaylist);
    bgVideoPlayer->setVideoOutput(bgVideoWidget);
    bgVideoPlayer->setMuted(true);                  // 배경음은 별도 mp3로 재생
    bgVideoWidget->setAspectRatioMode(Qt::KeepAspectRatioByExpanding);
    bgVideoPlayer->play();

    // Buttons
    dbButton    = new QPushButton("DB조회/서버통신", menuPage);
    graphButton = new QPushButton("3D 탐지 현황",   menuPage);

    const QString buttonStyle =
        "QPushButton { border: 1px solid #8f8f91; border-radius: 6px; "
        "background-color: qlineargradient(x1:0,y1:0,x2:0,y2:1, stop:0 #f6f7fa, stop:1 #dadbde); "
        "min-width: 120px; padding: 10px; } "
        "QPushButton:pressed { background-color: qlineargradient(x1:0,y1:0,x2:0,y2:1, stop:0 #dadbde, stop:1 #f6f7fa); }";
    dbButton->setStyleSheet(buttonStyle);
    graphButton->setStyleSheet(buttonStyle);

    auto *buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    buttonLayout->addWidget(dbButton);
    buttonLayout->addWidget(graphButton);
    buttonLayout->addStretch();

    // Assemble menu page
    menuLayout->addWidget(titleLabel);
    menuLayout->addWidget(bgVideoWidget, /*stretch*/ 1);
    menuLayout->addLayout(buttonLayout);
    menuPage->setLayout(menuLayout);

    // --- Menu BGM: 메뉴 들어올 때마다 1회만 재생 ---
    menuBgmPlayer = new QMediaPlayer(menuPage);
    const QString bgmPath = "/home/ubuntu/Intel_AI_Project_Team02/Qt/Intel_AI_Qt/scv-good-to-go-sir-101soundboards.mp3";
    if (!QFileInfo::exists(bgmPath)) {
        qWarning() << "[BGM] not found:" << bgmPath;
    }
    menuBgmPlayer->setMedia(QUrl::fromLocalFile(bgmPath)); // Qt5 API
    menuBgmPlayer->setVolume(40); // 0~100

    // --- Create Tab Pages ---
    pTab1Data   = new Tab1Data();
    pTab3DGraph = new Tab3DGraph();

    // --- Add pages to Stacked Widget ---
    stackedWidget->addWidget(menuPage);    // 0
    stackedWidget->addWidget(pTab1Data);   // 1
    stackedWidget->addWidget(pTab3DGraph); // 2

    mainLayout->addWidget(stackedWidget);
    this->setLayout(mainLayout);

    // --- Initial page ---
    stackedWidget->setCurrentIndex(0);

    // 메뉴 첫 진입 시 1회 재생
    menuBgmPlayer->setPosition(0);
    menuBgmPlayer->play();

    // --- Navigation ---
    connect(dbButton,    &QPushButton::clicked, this, &MainWidget::showDBPage);
    connect(graphButton, &QPushButton::clicked, this, &MainWidget::showGraphPage);

    connect(pTab1Data,   &Tab1Data::menuButtonClicked,  this, &MainWidget::showMenuPage);
    connect(pTab3DGraph, &Tab3DGraph::menuButtonClicked,this, &MainWidget::showMenuPage);

    // Data flow: Tab1 → Tab3DGraph
    connect(pTab1Data, &Tab1Data::droneDetected,
            pTab3DGraph, &Tab3DGraph::addDetection);
}

void MainWidget::showDBPage()
{
    stackedWidget->setCurrentIndex(1);
    if (menuBgmPlayer) menuBgmPlayer->stop();  // 메뉴 벗어나면 BGM 정지
}

void MainWidget::showGraphPage()
{
    stackedWidget->setCurrentIndex(2);
    if (menuBgmPlayer) menuBgmPlayer->stop();
}

void MainWidget::showMenuPage()
{
    stackedWidget->setCurrentIndex(0);
    if (menuBgmPlayer) {                       // 메뉴 들어올 때마다 “한 번” 재생
        menuBgmPlayer->setPosition(0);
        menuBgmPlayer->play();
    }
}
