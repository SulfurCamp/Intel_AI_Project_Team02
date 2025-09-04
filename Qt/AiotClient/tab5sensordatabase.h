#ifndef TAB5SENSORDATABASE_H
#define TAB5SENSORDATABASE_H

#include <QWidget>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDateTime>
#include <QDebug>
#include <QChartView>
#include <QLineSeries>
#include <QDateTimeAxis>
#include <QValueAxis>

#include <QTableWidgetItem>

#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
QT_CHARTS_USE_NAMESPACE
#endif

#define PROFESSOR_CODE 0

namespace Ui {
class Tab5sensordatabase;
}

class Tab5sensordatabase : public QWidget
{
    Q_OBJECT

public:
    explicit Tab5sensordatabase(QWidget *parent = nullptr);
    ~Tab5sensordatabase();

private:
    Ui::Tab5sensordatabase *ui;
    QSqlDatabase qSqlDatabase;
    QLineSeries * illuline; // 조도
    QLineSeries * humi; // 습도
    QLineSeries * temp; // 온도
    QChart * pQChart;
    QChartView * pQChartView;
    QDateTimeAxis *pQDateTimeAxisX; // 좌표 사용
#if PROFESSOR_CODE
    QTableWidgetItem* pQTableWidgetItemId = nullptr;
    QTableWidgetItem* pQTableWidgetItemDate = nullptr;
    QTableWidgetItem* pQTableWidgetItemIllu = nullptr;
#endif

    void updateLastDateTimeSql(bool);

private slots:
    void Tab5RecvDataSlot(QString);
    void on_pushButton_clicked();
    void on_pPBsearchDB_clicked();
    void on_pPBdeleteDB_clicked();
};

#endif // TAB5SENSORDATABASE_H
