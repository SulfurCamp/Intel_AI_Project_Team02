#pragma once
#include <QWidget>
#include <QDateTime>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include "sockclient.h"

namespace Ui {
    class Tab1Data;
}

class Tab1Data : public QWidget {
    Q_OBJECT
public:
    explicit Tab1Data(QWidget* parent=nullptr);
    ~Tab1Data();

signals:
    void menuButtonClicked();
    void droneDetected(double x, double y, double z);

private slots:
    // 상단 버튼
    void on_pPBQuery_clicked();
    void on_pPBServerConnect_toggled(bool checked);
    // 터미널/송수신
    void on_pPBSend_clicked();
    void onRecvLine(QString line);
    void onSocketConnected();
    void onSocketDisconnected();
    void onSocketError(QString msg);

private:
    Ui::Tab1Data* ui = nullptr;
    SockClient* sock = nullptr;

    // DB(sqlite)
    QSqlDatabase db;
    void ensureDb();
    void insertEvent(double x, double y, double z);
    void loadSnapshotAndRender();

    // 헬퍼
    void appendTerminal(const QString& line);
    void setupTableHeader();
    void tryIngestFromLine(const QString& line); // "640@320@0.826@drone" 등 파싱
};
