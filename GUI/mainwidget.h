#pragma once
#include <QWidget>
#include <QBuffer>
#include <QTemporaryFile>
#include <QScopedPointer>

class QStackedWidget;
class QLabel;
class QPushButton;
class QMediaPlayer;
class QMediaPlaylist;
class QVideoWidget;
class Tab1Data;
class Tab3DGraph;

class MainWidget : public QWidget
{
    Q_OBJECT
public:
    explicit MainWidget(QWidget *parent = nullptr);
    ~MainWidget();

private slots:
    void showDBPage();
    void showGraphPage();
    void showMenuPage();

private:
    void setupUi();

    // UI
    QStackedWidget *stackedWidget = nullptr;
    QWidget        *menuPage      = nullptr;
    QLabel         *titleLabel    = nullptr;
    QPushButton    *dbButton      = nullptr;
    QPushButton    *graphButton   = nullptr;

    // Pages
    Tab1Data      *pTab1Data   = nullptr;
    Tab3DGraph    *pTab3DGraph = nullptr;

    // Menu background video
    QVideoWidget  *bgVideoWidget   = nullptr;
    QMediaPlayer  *bgVideoPlayer   = nullptr;
    QMediaPlaylist*bgVideoPlaylist = nullptr;

    // Menu BGM (enter menu → play once)
    QMediaPlayer  *menuBgmPlayer   = nullptr;

    QScopedPointer<QTemporaryFile> m_bgTmp; // 배경영상 임시파일
    QByteArray m_bgmBytes;                  // 메뉴 BGM 바이트
    QBuffer*   m_bgmBuf = nullptr;          // 메뉴 BGM 버퍼

};
