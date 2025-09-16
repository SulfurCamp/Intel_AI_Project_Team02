#ifndef TAB3DGRAPH_H
#define TAB3DGRAPH_H

#include <QWidget>
#include <QTimer>
#include <QVector>
#include <QtDataVisualization/Q3DScatter>
#include <QtDataVisualization/QScatter3DSeries>
#include <QtDataVisualization/QScatterDataProxy>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QMediaPlayer>

using namespace QtDataVisualization;

class Tab3DGraph : public QWidget
{
    Q_OBJECT
public:
    explicit Tab3DGraph(QWidget *parent = nullptr);
    ~Tab3DGraph();

public slots:
    void addDetection(double x, double y, double z);
    void reloadRecentFromDb(int limit = 20);

signals:
    void menuButtonClicked();

private slots:
    void presetFront();
    void presetSide();
    void presetTop();
    void presetIso();

private:
    Q3DScatter *m_graph = nullptr;
    QWidget *m_container = nullptr;

    // ★ 시리즈 4개: 빨강/노랑/파랑 점 + “선” 근사용(얇은 흰 점들)
    QScatter3DSeries *m_seriesRed   = nullptr;
    QScatter3DSeries *m_seriesYellow= nullptr;
    QScatter3DSeries *m_seriesBlue  = nullptr;
    QScatter3DSeries *m_seriesPath  = nullptr;

    QVector<QVector3D> m_points;   // 정규화된 최근 포인트들(최대 20)
    int  m_maxPoints   = 20;
    int  m_inputWidth  = 1280;
    int  m_inputHeight = 640;

    // “선” 근사 시 세그먼트당 보간 포인트 개수(크면 부드러운 선)
    int  m_lineInterp  = 8;

    void initializeGraph();
    QVector3D normalize(double x, double y, double z) const;
    void pushPoint(const QVector3D &p);
    void rebuildAll();     // 색상별 점 + 경로(“선”) 모두 갱신
    void rebuildPath();    // 경로(“선”)만 갱신

    QMediaPlayer* m_redAlertPlayer = nullptr;
};

#endif // TAB3DGRAPH_H
