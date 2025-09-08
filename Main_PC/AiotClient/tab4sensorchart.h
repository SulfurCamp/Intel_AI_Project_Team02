#ifndef TAB4SENSORCHART_H
#define TAB4SENSORCHART_H

#include <QWidget>
#include <QTime>
#include <QDate>
#include <QDebug>
#include <QChartView>
#include <QLineSeries>
#include <QDateTimeAxis>
#include <QValueAxis>
#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
QT_CHARTS_USE_NAMESPACE
#endif

namespace Ui {
class Tab4SensorChart;
}

class Tab4SensorChart : public QWidget
{
    Q_OBJECT

public:
    explicit Tab4SensorChart(QWidget *parent = nullptr);
    ~Tab4SensorChart();

private:
    Ui::Tab4SensorChart *ui;
    QLineSeries * illuline; // 조도
    QLineSeries * humi; // 습도
    QLineSeries * temp; // 온도
    QChart * pQChart;
    QChartView * pQChartView;
    QDateTimeAxis *pQDateTimeAxisX; // 좌표 사용
    QDateTime firstDateTime;
    QDateTime lastDateTime;

    void updateLastDateTime(bool);

private slots:
    void Tab4RecvDataSlot(QString);
    void on_pPBClearChart_clicked();
};

#endif // TAB4SENSORCHART_H
