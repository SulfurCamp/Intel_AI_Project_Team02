#pragma once
#include <QWidget>
#include <QDateTime>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include "sockclient.h"

namespace Ui
{
    class pTabWidget;
}

class Tab1Data : public QWidget {
    Q_OBJECT
public:
    explicit Tab1Data(QWidget* parent=nullptr);
    ~Tab1Data();

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
    Ui::pTabWidget* ui = nullptr;
    SockClient* sock = nullptr;

    // DB(sqlite)
    QSqlDatabase db;
    void ensureDb();
    void insertEvent(const QString& locate, int distance);
    void loadSnapshotAndRender();

    // 헬퍼
    void appendTerminal(const QString& line);
    void setupTableHeader();
    void tryIngestFromLine(const QString& line); // "A@1" 등 파싱
};