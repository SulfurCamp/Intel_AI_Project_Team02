#include "tab3dgraph.h"

#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtGui/QScreen>
#include <QtCore/QRandomGenerator>
#include <QtCore/QDebug>
#include <QtCore/QFile>
#include <QtCore/QBuffer>
#include <QtMultimedia/QMediaContent>
#include <QtSql/QSqlError>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <algorithm>

using namespace QtDataVisualization;

Tab3DGraph::Tab3DGraph(QWidget *parent) : QWidget(parent)
{
    initializeGraph();

    // ───────── 상단 툴바 ─────────
    auto *btnFront = new QPushButton("Front", this);
    auto *btnSide  = new QPushButton("Side", this);
    auto *btnTop   = new QPushButton("Top", this);
    auto *btnIso   = new QPushButton("Iso", this);
    auto *btnMenu  = new QPushButton("Go to Menu", this);
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

    // ───────── 오른쪽 패널 ─────────
    auto *panel = new QWidget(this);
    auto *panelBox = new QVBoxLayout(panel);
    panelBox->setContentsMargins(8,8,8,8);
    panelBox->setSpacing(8);

    auto *gbStats = new QGroupBox("Situation", panel);
    auto *g = new QGridLayout(gbStats);
    m_lblCountRed   = new QLabel("Red: 0", gbStats);
    m_lblCountYel   = new QLabel("Yellow: 0", gbStats);
    m_lblCountBlue  = new QLabel("Blue: 0", gbStats);
    m_lblCountTotal = new QLabel("Total: 0", gbStats);
    m_lblRisk       = new QLabel("Thread Level: -", gbStats);
    m_lblRisk->setStyleSheet("QLabel{font-weight:bold;color:#ffd54f;}");
    g->addWidget(m_lblCountRed,   0, 0);
    g->addWidget(m_lblCountYel,   0, 1);
    g->addWidget(m_lblCountBlue,  1, 0);
    g->addWidget(m_lblCountTotal, 1, 1);
    g->addWidget(m_lblRisk,       2, 0, 1, 2);
    gbStats->setLayout(g);

    auto *gbControls = new QGroupBox("Dp/Control", panel);
    auto *h = new QHBoxLayout(gbControls);
    m_btnTogglePath = new QPushButton("Path", gbControls);
    m_btnTogglePath->setCheckable(true);
    m_btnTogglePath->setChecked(true);
    m_btnClear = new QPushButton("Init", gbControls);
    m_btnToggleDb = new QPushButton("Past Report", gbControls);
    m_btnToggleDb->setCheckable(true);
    h->addWidget(m_btnTogglePath);
    h->addWidget(m_btnToggleDb);
    h->addStretch(1);
    h->addWidget(m_btnClear);
    gbControls->setLayout(h);

    panelBox->addWidget(gbStats);
    panelBox->addWidget(gbControls);
    panelBox->addStretch(1);

    connect(m_btnTogglePath, &QPushButton::toggled, this, &Tab3DGraph::onTogglePath);
    connect(m_btnClear,      &QPushButton::clicked, this, &Tab3DGraph::onClearClicked);
    connect(m_btnToggleDb,   &QPushButton::toggled, this, &Tab3DGraph::onToggleDbOverlay);

    // ───────── 전체 레이아웃 ─────────
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addLayout(toolBar);

    auto *content = new QHBoxLayout();
    content->setContentsMargins(0,0,0,0);
    content->addWidget(m_container, /*stretch*/ 1);
    content->addWidget(panel);
    mainLayout->addLayout(content, /*stretch*/ 1);
    setLayout(mainLayout);

    // 필요 시 시작할 때 DB에서 불러오기
    // reloadRecentFromDb(20);
}

Tab3DGraph::~Tab3DGraph() {}

// ─────────────────────────────────────────────────────────────────────

void Tab3DGraph::initializeGraph()
{
    m_graph = new Q3DScatter();
    m_container = QWidget::createWindowContainer(m_graph);

    // 축
    m_graph->axisX()->setTitle("X");  m_graph->axisX()->setTitleVisible(true);  m_graph->axisX()->setRange(-1.0f, 1.0f);
    m_graph->axisY()->setTitle("Y");  m_graph->axisY()->setTitleVisible(true);  m_graph->axisY()->setRange(0.0f, 1.0f);
    m_graph->axisZ()->setTitle("Depth (Z, m)"); m_graph->axisZ()->setTitleVisible(true); m_graph->axisZ()->setRange(0.0f, 1.5f);
    m_graph->setShadowQuality(QAbstract3DGraph::ShadowQualityNone);
    m_graph->setReflection(false);

    // 테마(다크)
    auto *theme = new Q3DTheme(Q3DTheme::ThemeEbony);
    m_graph->setActiveTheme(theme);
    theme->setBackgroundEnabled(true);
    theme->setBackgroundColor(Qt::black);
    theme->setWindowColor(Qt::black);
    theme->setLabelTextColor(Qt::white);
    theme->setGridLineColor(QColor(70,70,70));
    theme->setLabelBorderEnabled(false);
    m_container->setAutoFillBackground(true);
    QPalette pal = m_container->palette(); pal.setColor(QPalette::Window, Qt::black); m_container->setPalette(pal);

    // 시리즈(실시간)
    const float pointSize = 0.22f;
    m_seriesRed    = new QScatter3DSeries();   m_seriesRed->setBaseColor(QColor("#e53935")); m_seriesRed->setItemSize(pointSize);
    m_seriesYellow = new QScatter3DSeries();   m_seriesYellow->setBaseColor(QColor("#fdd835")); m_seriesYellow->setItemSize(pointSize);
    m_seriesBlue   = new QScatter3DSeries();   m_seriesBlue->setBaseColor(QColor("#1e88e5")); m_seriesBlue->setItemSize(pointSize);
    m_seriesPath   = new QScatter3DSeries();   m_seriesPath->setBaseColor(QColor("#f5f5f5"));  m_seriesPath->setItemSize(0.06f);

    QString lbl = QStringLiteral("x:@xLabel y:@yLabel z:@zLabel");
    m_seriesRed->setItemLabelFormat(lbl);
    m_seriesYellow->setItemLabelFormat(lbl);
    m_seriesBlue->setItemLabelFormat(lbl);

    // 시리즈(DB 오버레이; 작고 옅게)
    const float dbPointSize = 0.12f;
    m_seriesDbRed    = new QScatter3DSeries(); m_seriesDbRed->setBaseColor(QColor(229,57,53,140));   m_seriesDbRed->setItemSize(dbPointSize);
    m_seriesDbYellow = new QScatter3DSeries(); m_seriesDbYellow->setBaseColor(QColor(253,216,53,140));m_seriesDbYellow->setItemSize(dbPointSize);
    m_seriesDbBlue   = new QScatter3DSeries(); m_seriesDbBlue->setBaseColor(QColor(30,136,229,140));  m_seriesDbBlue->setItemSize(dbPointSize);

    // 그래프에 추가 (경로 → 실시간 → DB)
    m_graph->addSeries(m_seriesPath);
    m_graph->addSeries(m_seriesRed);
    m_graph->addSeries(m_seriesYellow);
    m_graph->addSeries(m_seriesBlue);
    m_graph->addSeries(m_seriesDbRed);
    m_graph->addSeries(m_seriesDbYellow);
    m_graph->addSeries(m_seriesDbBlue);

    m_seriesDbRed->setVisible(false);
    m_seriesDbYellow->setVisible(false);
    m_seriesDbBlue->setVisible(false);

    presetFront();

    // ── 빨강 포인트 알림음 (qrc → QBuffer 재생) ──
    m_redAlertPlayer = new QMediaPlayer(this);
    {
        QFile rf(":/media/base-is-under-attack-101soundboards.mp3");
        if (rf.open(QIODevice::ReadOnly)) {
            m_redBytes = rf.readAll();
            if (!m_redBuf) m_redBuf = new QBuffer(this);
            m_redBuf->setData(m_redBytes);
            m_redBuf->open(QIODevice::ReadOnly);
            m_redAlertPlayer->setMedia(QMediaContent(), m_redBuf); // Qt5 API
            m_redAlertPlayer->setVolume(80);
        } else {
            qWarning() << "[RED-SFX] qrc not found :/media/base-is-under-attack-101soundboards.mp3";
        }
    }

    // --- 최신점(헤드) 시리즈: 순서 시각화 ---
    m_seriesLatest = new QScatter3DSeries();
    m_seriesLatest->setBaseColor(QColor("#00E5FF")); // 시안색
    m_seriesLatest->setItemSize(0.30f);
    m_graph->addSeries(m_seriesLatest); // 맨 마지막에 추가해서 위에 보이게

    // 경로(선 느낌) 보강
    m_lineInterp = 10;
    m_seriesPath->setVisible(true);
}

// ─────────────────────────────────────────────────────────────────────

QVector3D Tab3DGraph::normalize(double x, double y, double z) const
{
    double X = qBound(0.0, x, double(m_inputWidth));
    float nx = float((X / double(m_inputWidth)) * 2.0 - 1.0); nx = qBound(-1.0f, nx, 1.0f);
    double Y = qBound(0.0, y, double(m_inputHeight));
    float ny = float(Y / double(m_inputHeight));               ny = qBound(0.0f, ny, 1.0f);
    float nz = float(z);
    return QVector3D(nx, ny, nz);
}

void Tab3DGraph::pushPoint(const QVector3D &p)
{
    const int before = m_points.size();
    m_points.push_back(p);

    // 안전 캡: 30개 초과분 전부 제거
    while (m_points.size() > m_maxPoints) {
        m_points.remove(0);
    }

    if (m_points.size() != before + 1) {
        qDebug() << "[3D] Trimmed oldest. size =" << m_points.size();
    }

    rebuildAll();
}

void Tab3DGraph::rebuildAll()
{
    // 혹시라도 외부에서 크기가 늘어났다면 마지막 30개만 유지
    if (m_points.size() > m_maxPoints)
        m_points = m_points.mid(m_points.size() - m_maxPoints);

    // 색상별 분배
    auto *arrRed    = new QScatterDataArray;
    auto *arrYellow = new QScatterDataArray;
    auto *arrBlue   = new QScatterDataArray;
    arrRed->reserve(m_points.size());
    arrYellow->reserve(m_points.size());
    arrBlue->reserve(m_points.size());

    for (const auto &pt : m_points) {
        const float z = pt.z();
        if (z <= 0.4f)       arrRed->append(QScatterDataItem(pt));
        else if (z <= 0.7f)  arrYellow->append(QScatterDataItem(pt));
        else                 arrBlue->append(QScatterDataItem(pt));
    }

    m_seriesRed->dataProxy()->resetArray(arrRed);
    m_seriesYellow->dataProxy()->resetArray(arrYellow);
    m_seriesBlue->dataProxy()->resetArray(arrBlue);

    // 경로 갱신
    rebuildPath();

    // 통계/리스크 UI 갱신
    updateStats(arrRed->size(), arrYellow->size(), arrBlue->size());
    updateRiskLabel();

    // 최신점(헤드) 표시
    if (!m_points.isEmpty() && m_seriesLatest) {
        auto* arrLatest = new QScatterDataArray;
        arrLatest->append(QScatterDataItem(m_points.last()));
        m_seriesLatest->dataProxy()->resetArray(arrLatest);
    }
}

void Tab3DGraph::rebuildPath()
{
    auto *arrPath = new QScatterDataArray;
    if (m_points.size() >= 2) {
        arrPath->reserve((m_points.size() - 1) * m_lineInterp);
        for (int i = 1; i < m_points.size(); ++i) {
            const QVector3D a = m_points[i-1];
            const QVector3D b = m_points[i];
            for (int k = 1; k <= m_lineInterp; ++k) {
                float t = float(k) / float(m_lineInterp);
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

    // z <= 0.4 → 알림음
    if (p.z() <= 0.4f && m_redAlertPlayer) {
        if (m_redAlertPlayer->state() == QMediaPlayer::PlayingState) m_redAlertPlayer->setPosition(0);
        else m_redAlertPlayer->play();
    }

    pushPoint(p);
}

// ─────────────────────────────────────────────────────────────────────

void Tab3DGraph::updateStats(int red, int yellow, int blue)
{
    const int total = red + yellow + blue;
    if (m_lblCountRed)   m_lblCountRed->setText(QString("Red: %1").arg(red));
    if (m_lblCountYel)   m_lblCountYel->setText(QString("Yellow: %1").arg(yellow));
    if (m_lblCountBlue)  m_lblCountBlue->setText(QString("Blue: %1").arg(blue));
    if (m_lblCountTotal) m_lblCountTotal->setText(QString("Total: %1").arg(total));
}

void Tab3DGraph::updateRiskLabel()
{
    QString level = "Safe";
    QString css   = "color:#80cbc4;font-weight:bold;"; // teal

    const int n = m_points.size();
    if (n == 0) { if (m_lblRisk) m_lblRisk->setText("Thread Level: -"); return; }

    // 빨강 개수
    int red = 0;
    for (const auto &pt : m_points) if (pt.z() <= 0.4f) ++red;

    // 최근 5개 Z 하강 추세(접근)
    int k = qMin(5, n);
    int decreasing = 0;
    for (int i = n-k+1; i < n; ++i) {
        if (i <= 0) continue;
        if (m_points[i].z() < m_points[i-1].z() - 0.02f) ++decreasing; // 임계값 0.02
    }

    if (red >= 5) { level = "High Thread"; css = "color:#ff5252;font-weight:bold;"; }
    else if (decreasing >= 3) { level = "Approach"; css = "color:#ffd54f;font-weight:bold;"; }

    if (m_lblRisk) {
        m_lblRisk->setText(QString("Thread Level: %1").arg(level));
        m_lblRisk->setStyleSheet("QLabel{"+css+"}");
    }
}

// ─────────────────────────────────────────────────────────────────────
// DB

void Tab3DGraph::reloadRecentFromDb(int limit)
{
    const QString cname = "events_conn";
    if (!QSqlDatabase::contains(cname)) { qWarning() << "[3D] DB connection not found:" << cname; return; }
    QSqlDatabase db = QSqlDatabase::database(cname);
    if (!db.isOpen()) { qWarning() << "[3D] DB not open"; return; }

    QSqlQuery q(db);
    q.prepare(QStringLiteral("SELECT x,y,z FROM events ORDER BY id DESC LIMIT %1").arg(limit));
    if (!q.exec()) { qWarning() << "[3D] DB query fail:" << q.lastError().text(); return; }

    QVector<QVector3D> tmp;
    while (q.next()) {
        const double x = q.value(0).toDouble();
        const double y = q.value(1).toDouble();
        const double z = q.value(2).toDouble();
        tmp.push_back(normalize(x, y, z));
    }
    std::reverse(tmp.begin(), tmp.end());
    m_points = (tmp.size() > m_maxPoints) ? tmp.mid(tmp.size() - m_maxPoints) : tmp;
    rebuildAll();
}

QVector<QVector3D> Tab3DGraph::fetchRecentFromDb(int limit)
{
    QVector<QVector3D> out;
    const QString cname = "events_conn";
    if (!QSqlDatabase::contains(cname)) { qWarning() << "[3D] DB connection not found:" << cname; return out; }
    QSqlDatabase db = QSqlDatabase::database(cname);
    if (!db.isOpen()) { qWarning() << "[3D] DB not open"; return out; }

    QSqlQuery q(db);
    q.prepare(QStringLiteral("SELECT x,y,z FROM events ORDER BY id DESC LIMIT %1").arg(limit));
    if (!q.exec()) { qWarning() << "[3D] DB query fail:" << q.lastError().text(); return out; }

    while (q.next()) {
        const double x = q.value(0).toDouble();
        const double y = q.value(1).toDouble();
        const double z = q.value(2).toDouble();
        out.push_back(normalize(x, y, z));
    }
    std::reverse(out.begin(), out.end());
    return out;
}

// ─────────────────────────────────────────────────────────────────────
// 패널 핸들러

void Tab3DGraph::onTogglePath(bool on)
{
    if (m_seriesPath) m_seriesPath->setVisible(on);
}

void Tab3DGraph::onClearClicked()
{
    m_points.clear();
    rebuildAll();
}

void Tab3DGraph::onToggleDbOverlay(bool on)
{
    m_dbOverlayOn = on;
    m_btnToggleDb->setText(on ? "Hide Past Report" : "Review Past Report");
    m_seriesDbRed->setVisible(on);
    m_seriesDbYellow->setVisible(on);
    m_seriesDbBlue->setVisible(on);

    if (on) refreshDbOverlay();
    else {
        // 비활성 시 배열 비움
        m_seriesDbRed->dataProxy()->resetArray(new QScatterDataArray);
        m_seriesDbYellow->dataProxy()->resetArray(new QScatterDataArray);
        m_seriesDbBlue->dataProxy()->resetArray(new QScatterDataArray);
    }
}

void Tab3DGraph::refreshDbOverlay()
{
    m_dbPoints = fetchRecentFromDb(100); // 필요 시 숫자 조절

    auto *arrR = new QScatterDataArray;
    auto *arrY = new QScatterDataArray;
    auto *arrB = new QScatterDataArray;
    arrR->reserve(m_dbPoints.size());
    arrY->reserve(m_dbPoints.size());
    arrB->reserve(m_dbPoints.size());

    for (const auto &pt : m_dbPoints) {
        const float z = pt.z();
        if (z <= 0.4f)       arrR->append(QScatterDataItem(pt));
        else if (z <= 0.7f)  arrY->append(QScatterDataItem(pt));
        else                 arrB->append(QScatterDataItem(pt));
    }

    m_seriesDbRed->dataProxy()->resetArray(arrR);
    m_seriesDbYellow->dataProxy()->resetArray(arrY);
    m_seriesDbBlue->dataProxy()->resetArray(arrB);
}

// ─────────────────────────────────────────────────────────────────────
// 뷰 프리셋

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
