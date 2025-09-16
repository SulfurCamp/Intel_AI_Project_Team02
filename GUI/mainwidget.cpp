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

#include <QFile>
#include <QDir>
#include <QStandardPaths>
#include <QBuffer>
#include <QTemporaryFile>

MainWidget::MainWidget(QWidget *parent)
    : QWidget(parent)
{
    setupUi();
}

MainWidget::~MainWidget()
{
    // Auto-deleted by parent-child destructor rule
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

    // In Qt5, direct playback of qrc: mp4 sometimes fails
    // qrc -> copy to temporary mp4 file -> play from local path (most compatible)
    {
        QFile vres(":/media/background.mp4");
        if (vres.open(QIODevice::ReadOnly)) {
            static QTemporaryFile* s_bgTmp = nullptr;
            if (!s_bgTmp) {
                s_bgTmp = new QTemporaryFile(
                    QDir(QStandardPaths::writableLocation(QStandardPaths::TempLocation))
                        .filePath("bg_XXXXXX.mp4"));
                s_bgTmp->setAutoRemove(true);
                if (s_bgTmp->open()) {
                    s_bgTmp->write(vres.readAll());
                    s_bgTmp->flush();
                } else {
                    qWarning() << "[VIDEO] temp file open failed";
                }
            }
            bgVideoPlaylist->clear();
            bgVideoPlaylist->addMedia(QUrl::fromLocalFile(s_bgTmp->fileName()));
            bgVideoPlaylist->setPlaybackMode(QMediaPlaylist::Loop);
            bgVideoPlayer->setPlaylist(bgVideoPlaylist);
            bgVideoPlayer->setVideoOutput(bgVideoWidget);
            bgVideoWidget->setAspectRatioMode(Qt::KeepAspectRatioByExpanding);
            bgVideoPlayer->setMuted(true);                  // Background music is played as a separate mp3
            bgVideoPlayer->play();
        } else {
            qWarning() << "[VIDEO] qrc not found :/media/background.mp4";
        }
    }

    // Buttons
    dbButton    = new QPushButton("DB/Server", menuPage);
    graphButton = new QPushButton("3D Detection Status",   menuPage);

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

    // --- Menu BGM: Play only once every time you enter the menu ---
    menuBgmPlayer = new QMediaPlayer(menuPage);

    // qrc -> QBuffer(QIODevice) playback (safe in Qt5)
    {
        QFile fres(":/media/scv-good-to-go-sir-101soundboards.mp3");
        if (fres.open(QIODevice::ReadOnly)) {
            static QByteArray s_bgmBytes;
            static QBuffer*   s_bgmBuf = nullptr;
            s_bgmBytes = fres.readAll();
            if (!s_bgmBuf) s_bgmBuf = new QBuffer(menuPage);
            s_bgmBuf->setData(s_bgmBytes);
            s_bgmBuf->open(QIODevice::ReadOnly);

            menuBgmPlayer->setMedia(QMediaContent(), s_bgmBuf);
            menuBgmPlayer->setVolume(60); // A little louder
        } else {
            qWarning() << "[BGM] qrc not found :/media/scv-good-to-go-sir-101soundboards.mp3";
        }

        // Error/status log (check cause immediately)
        connect(menuBgmPlayer, QOverload<QMediaPlayer::Error>::of(&QMediaPlayer::error),
                this, [this](QMediaPlayer::Error e){
                    qWarning() << "[BGM] error =" << e << menuBgmPlayer->errorString();
                });
        connect(menuBgmPlayer, &QMediaPlayer::mediaStatusChanged, this,
                [](QMediaPlayer::MediaStatus s){ qDebug() << "[BGM] status" << s; });
    }

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

    // Play once when entering the menu for the first time
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
    if (menuBgmPlayer) menuBgmPlayer->stop();  // Stop BGM when leaving the menu
}

void MainWidget::showGraphPage()
{
    stackedWidget->setCurrentIndex(2);
    if (menuBgmPlayer) menuBgmPlayer->stop();
}

void MainWidget::showMenuPage()
{
    stackedWidget->setCurrentIndex(0);
    if (menuBgmPlayer) {                       // Play "once" every time you enter the menu
        menuBgmPlayer->setPosition(0);
        menuBgmPlayer->play();
    }
}
