#pragma once
#include <QWidget>

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
};
