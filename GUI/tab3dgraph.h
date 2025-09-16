#pragma once

#include <QWidget>
#include <QVector>
#include <QVector3D>
#include <QLabel>
#include <QPushButton>
#include <QMediaPlayer>

#include <QtDataVisualization/Q3DScatter>
#include <QtDataVisualization/QScatter3DSeries>
#include <QtDataVisualization/Q3DCamera>

class QBuffer; // qrc 오디오 재생용

class Tab3DGraph : public QWidget
{
    Q_OBJECT
public:
    explicit Tab3DGraph(QWidget *parent = nullptr);
    ~Tab3DGraph();

signals:
    void menuButtonClicked();

public slots:
    void addDetection(double x, double y, double z);
    void presetFront();
    void presetSide();
    void presetTop();
    void presetIso();

private:
    // 초기화 & 렌더링
    void initializeGraph();
    void rebuildAll();
    void rebuildPath();
    QVector3D normalize(double x, double y, double z) const;

    void pushPoint(const QVector3D &p);

    // DB
    void reloadRecentFromDb(int limit);              // 실시간 세트 교체용
    QVector<QVector3D> fetchRecentFromDb(int limit); // 오버레이용

    // 통계/분석/패널
    void updateStats(int red, int yellow, int blue);
    void updateRiskLabel();

    // 패널 핸들러
    void onTogglePath(bool on);
    void onClearClicked();
    void onToggleDbOverlay(bool on);
    void refreshDbOverlay(); // 켜진 상태에서 다시 로드

private:
    // 3D 그래프
    QtDataVisualization::Q3DScatter *m_graph = nullptr;
    QWidget *m_container = nullptr;

    // 시리즈(실시간)
    QtDataVisualization::QScatter3DSeries *m_seriesRed = nullptr;
    QtDataVisualization::QScatter3DSeries *m_seriesYellow = nullptr;
    QtDataVisualization::QScatter3DSeries *m_seriesBlue = nullptr;
    QtDataVisualization::QScatter3DSeries *m_seriesPath = nullptr;

    // 최신점(헤드) - 순서 확인용
    QtDataVisualization::QScatter3DSeries* m_seriesLatest = nullptr;

    // 시리즈(DB 오버레이; 작고 옅게)
    QtDataVisualization::QScatter3DSeries *m_seriesDbRed = nullptr;
    QtDataVisualization::QScatter3DSeries *m_seriesDbYellow = nullptr;
    QtDataVisualization::QScatter3DSeries *m_seriesDbBlue = nullptr;
    bool m_dbOverlayOn = false;
    QVector<QVector3D> m_dbPoints;

    // 데이터
    QVector<QVector3D> m_points;
    int m_maxPoints   = 30;
    int m_lineInterp  = 10;     // 선 느낌 강화
    int m_inputWidth  = 1280;
    int m_inputHeight = 640;

    // 사운드
    QMediaPlayer *m_redAlertPlayer = nullptr;
    QBuffer      *m_redBuf         = nullptr; // qrc MP3 버퍼
    QByteArray    m_redBytes;

    // 패널 UI
    QLabel *m_lblCountRed   = nullptr;
    QLabel *m_lblCountYel   = nullptr;
    QLabel *m_lblCountBlue  = nullptr;
    QLabel *m_lblCountTotal = nullptr;
    QLabel *m_lblRisk       = nullptr;
    QPushButton *m_btnTogglePath = nullptr;
    QPushButton *m_btnClear      = nullptr;
    QPushButton *m_btnToggleDb   = nullptr;
};
