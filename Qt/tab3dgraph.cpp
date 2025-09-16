#include "tab3dgraph.h"
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QPushButton>
#include <QtDataVisualization/Q3DTheme>
#include <QtDataVisualization/QValue3DAxis>
#include <QtDataVisualization/Q3DCamera>
#include <QtGui/QVector3D>
#include <QtCore/QRandomGenerator>
#include <QtGui/QScreen>
#include <QtCore/QDebug>
#include <QtSql/QSqlError>
#include <algorithm>
#include <QMediaPlayer>
#include <QFileInfo>
#include <QUrl>

Tab3DGraph::Tab3DGraph(QWidget *parent) : QWidget(parent)
{
    initializeGraph();

    // 컨트롤 바
    auto *btnFront = new QPushButton("Front", this);
    auto *btnSide  = new QPushButton("Side", this);
    auto *btnTop   = new QPushButton("Top", this);
    auto *btnIso   = new QPushButton("Iso", this);
    auto *btnMenu  = new QPushButton("메뉴로 돌아가기", this);
    connect(btnFront, &QPushButton::clicked, this, &Tab3DGraph::presetFront);
    connect(btnSide,  &QPushButton::clicked, this, &Tab3DGraph::presetSide);
    connect(btnTop,   &QPushButton::clicked, this, &Tab3DGraph::presetTop);
    connect(btnIso,   &QPushButton::clicked, this, &Tab3DGraph::presetIso);
    connect(btnMenu,  &QPushButton::clicked, this, &Tab3DGraph::menuButtonClicked);

    auto *toolBar = new QHBoxLayout();
    toolBar->setContentsMargins(0, 0, 0, 0);
    toolBar->addWidget(btnFront);
    toolBar->addWidget(btnSide);
    toolBar->addWidget(btnTop);
    toolBar->addWidget(btnIso);
    toolBar->addStretch(1);
    toolBar->addWidget(btnMenu);

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addLayout(toolBar);
    mainLayout->addWidget(m_container, 1);
    setLayout(mainLayout);

    // 필요 시 시작할 때 DB에서 불러오기
    // reloadRecentFromDb(20);
}

Tab3DGraph::~Tab3DGraph() { }

void Tab3DGraph::initializeGraph()
{
    m_graph = new Q3DScatter();
    m_container = QWidget::createWindowContainer(m_graph);

    // 축
    m_graph->axisX()->setTitle("X");
    m_graph->axisX()->setTitleVisible(true);
    m_graph->axisX()->setRange(-1.0f, 1.0f);
    m_graph->axisY()->setTitle("Y");
    m_graph->axisY()->setTitleVisible(true);
    m_graph->axisY()->setRange(0.0f, 1.0f);
    m_graph->axisZ()->setTitle("Depth (Z, m)");
    m_graph->axisZ()->setTitleVisible(true);
    m_graph->axisZ()->setRange(0.0f, 1.5f);

    m_graph->setShadowQuality(QAbstract3DGraph::ShadowQualityNone);
    m_graph->setReflection(false);

    // ★ 다크 테마(검정 배경)
    auto *theme = new Q3DTheme(Q3DTheme::ThemeEbony);
    m_graph->setActiveTheme(theme);
    theme->setBackgroundEnabled(true);
    theme->setBackgroundColor(Qt::black);
    theme->setWindowColor(Qt::black);
    theme->setLabelTextColor(Qt::white);
    theme->setGridLineColor(QColor(70,70,70));
    theme->setLabelBorderEnabled(false);
    m_container->setAutoFillBackground(true);
    QPalette pal = m_container->palette();
    pal.setColor(QPalette::Window, Qt::black);
    m_container->setPalette(pal);

    // ★ 시리즈 4개 생성
    m_seriesRed    = new QScatter3DSeries();   // z <= 0.4
    m_seriesYellow = new QScatter3DSeries();   // 0.4 < z <= 0.7
    m_seriesBlue   = new QScatter3DSeries();   // z > 0.7
    m_seriesPath   = new QScatter3DSeries();   // “선” 근사용(작은 흰 점들)

    // 포인트 크기/색
    const float pointSize = 0.22f; // 기존보다 큼
    m_seriesRed->setBaseColor(QColor("#e53935"));    // 빨강
    m_seriesRed->setItemSize(pointSize);

    m_seriesYellow->setBaseColor(QColor("#fdd835")); // 노랑
    m_seriesYellow->setItemSize(pointSize);

    m_seriesBlue->setBaseColor(QColor("#1e88e5"));   // 파랑
    m_seriesBlue->setItemSize(pointSize);

    m_seriesPath->setBaseColor(QColor("#f5f5f5"));   // 밝은 회색(선 느낌)
    m_seriesPath->setItemSize(0.06f);                // 매우 작게

    // 라벨 포맷(원하면 시리즈별로 조정)
    QString lbl = QStringLiteral("x:@xLabel y:@yLabel z:@zLabel");
    m_seriesRed->setItemLabelFormat(lbl);
    m_seriesYellow->setItemLabelFormat(lbl);
    m_seriesBlue->setItemLabelFormat(lbl);

    // 그래프에 추가
    m_graph->addSeries(m_seriesPath);  // 경로 먼저(뒤에 추가된 점이 위에 보임)
    m_graph->addSeries(m_seriesRed);
    m_graph->addSeries(m_seriesYellow);
    m_graph->addSeries(m_seriesBlue);

    presetFront();

    // ── 빨강 포인트 알림음 초기화 ─────────────────────────────
    m_redAlertPlayer = new QMediaPlayer(this);
    const QString redSfx = "/home/ubuntu/Intel_AI_Project_Team02/Qt/Intel_AI_Qt/base-is-under-attack-101soundboards.mp3";
    if (!QFileInfo::exists(redSfx)) {
        qWarning() << "[RED-SFX] file not found:" << redSfx;
    }
    m_redAlertPlayer->setMedia(QUrl::fromLocalFile(redSfx)); // Qt5 API
    m_redAlertPlayer->setVolume(70); // 0~100, 필요 시 조절


}

QVector3D Tab3DGraph::normalize(double x, double y, double z) const
{
    // X: [0..1280] → [-1..1]
    double X = qBound(0.0, x, double(m_inputWidth));
    float nx = float((X / double(m_inputWidth)) * 2.0 - 1.0);
    if (nx < -1.f) nx = -1.f; else if (nx > 1.f) nx = 1.f;

    // Y: [0..640] → [0..1] (뒤집지 않음: 0→0, 640→1)
    double Y = qBound(0.0, y, double(m_inputHeight));
    float ny = float(Y / double(m_inputHeight));
    if (ny < 0.f) ny = 0.f; else if (ny > 1.f) ny = 1.f;

    // Z: 그대로 (축 범위는 outside)
    float nz = float(z);
    return QVector3D(nx, ny, nz);
}

void Tab3DGraph::pushPoint(const QVector3D &p)
{
    m_points.push_back(p);
    if (m_points.size() > m_maxPoints) {
        m_points.remove(0);
    }
    rebuildAll();
}

void Tab3DGraph::rebuildAll()
{
    // 색상별 배열 생성
    auto *arrRed    = new QScatterDataArray;
    auto *arrYellow = new QScatterDataArray;
    auto *arrBlue   = new QScatterDataArray;
    arrRed->reserve(m_points.size());
    arrYellow->reserve(m_points.size());
    arrBlue->reserve(m_points.size());

    for (const auto &pt : m_points) {
        const float z = pt.z();
        if (z <= 0.4f) {
            arrRed->append(QScatterDataItem(pt));
        } else if (z <= 0.7f) {
            arrYellow->append(QScatterDataItem(pt));
        } else {
            arrBlue->append(QScatterDataItem(pt));
        }
    }

    m_seriesRed->dataProxy()->resetArray(arrRed);
    m_seriesYellow->dataProxy()->resetArray(arrYellow);
    m_seriesBlue->dataProxy()->resetArray(arrBlue);

    // 경로(“선”) 갱신
    rebuildPath();
}

void Tab3DGraph::rebuildPath()
{
    // 인접 점 사이를 m_lineInterp개로 보간해서 “선”처럼 보이게
    auto *arrPath = new QScatterDataArray;
    if (m_points.size() >= 2) {
        // 대략 (N-1)*m_lineInterp 개
        arrPath->reserve((m_points.size() - 1) * m_lineInterp);
        for (int i = 1; i < m_points.size(); ++i) {
            const QVector3D a = m_points[i-1];
            const QVector3D b = m_points[i];
            for (int k = 1; k <= m_lineInterp; ++k) {
                float t = float(k) / float(m_lineInterp); // (0,1] 구간
                QVector3D p = (1.0f - t) * a + t * b;
                arrPath->append(QScatterDataItem(p));
            }
        }
    }
    m_seriesPath->dataProxy()->resetArray(arrPath);
}

void Tab3DGraph::addDetection(double x, double y, double z)
{
    const QVector3D p = normalize(x, y, z);

    // ✅ z <= 0.4 → 빨강 구간 = 알림음 재생
    if (p.z() <= 0.4f && m_redAlertPlayer) {
        if (m_redAlertPlayer->state() == QMediaPlayer::PlayingState) {
            // 짧은 효과음을 매번 처음부터 들리게
            m_redAlertPlayer->setPosition(0);
        } else {
            m_redAlertPlayer->play();
        }
    }

    pushPoint(p);
}

void Tab3DGraph::reloadRecentFromDb(int limit)
{
    const QString cname = "events_conn";
    if (!QSqlDatabase::contains(cname)) {
        qWarning() << "[3D] DB connection not found:" << cname;
        return;
    }
    QSqlDatabase db = QSqlDatabase::database(cname);
    if (!db.isOpen()) {
        qWarning() << "[3D] DB not open";
        return;
    }

    QSqlQuery q(db);
    q.prepare(QStringLiteral(
                  "SELECT x, y, z FROM events ORDER BY id DESC LIMIT %1").arg(limit));
    if (!q.exec()) {
        qWarning() << "[3D] DB query fail:" << q.lastError().text();
        return;
    }

    QVector<QVector3D> tmp;
    while (q.next()) {
        const double x = q.value(0).toDouble();
        const double y = q.value(1).toDouble();
        const double z = q.value(2).toDouble();
        tmp.push_back(normalize(x, y, z));
    }
    std::reverse(tmp.begin(), tmp.end());

    m_points = tmp;
    if (m_points.size() > m_maxPoints)
        m_points = m_points.mid(m_points.size() - m_maxPoints);

    rebuildAll();
}

/* ---- 뷰 프리셋 ---- */
void Tab3DGraph::presetFront()
{
    m_graph->scene()->activeCamera()->setCameraPreset(Q3DCamera::CameraPresetFront);
    m_graph->scene()->activeCamera()->setZoomLevel(120.f);
}
void Tab3DGraph::presetSide()
{
    m_graph->scene()->activeCamera()->setCameraPreset(Q3DCamera::CameraPresetRight);
    m_graph->scene()->activeCamera()->setZoomLevel(120.f);
}
void Tab3DGraph::presetTop()
{
    m_graph->scene()->activeCamera()->setCameraPreset(Q3DCamera::CameraPresetDirectlyAbove);
    m_graph->scene()->activeCamera()->setZoomLevel(120.f);
}
void Tab3DGraph::presetIso()
{
    m_graph->scene()->activeCamera()->setCameraPreset(Q3DCamera::CameraPresetIsometricRightHigh);
    m_graph->scene()->activeCamera()->setZoomLevel(120.f);
}
