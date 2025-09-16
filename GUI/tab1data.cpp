#include "tab1data.h"
#include "ui_tab1data.h"

#include <QMessageBox>
#include <QTableWidgetItem>
#include <QInputDialog>
#include <QStandardPaths>
#include <QDir>
#include <QSqlError>
#include <QMessageBox>

Tab1Data::Tab1Data(QWidget* parent)
    : QWidget(parent), ui(new Ui::Tab1Data)
{
    ui->setupUi(this);

    // 소켓
    sock = new SockClient(this);

    connect(sock, SIGNAL(socketRevcDataSig(QString)), this, SLOT(onRecvLine(QString)));
    connect(sock, &SockClient::connectedSig, this, &Tab1Data::onSocketConnected);
    connect(sock, &SockClient::disconnectedSig, this, &Tab1Data::onSocketDisconnected);
    connect(sock, &SockClient::errorSig, this, &Tab1Data::onSocketError);

    // 버튼 시그널
    connect(ui->pPBQuery, &QPushButton::clicked, this, &Tab1Data::on_pPBQuery_clicked);
    connect(ui->pPBServerConnect, &QPushButton::toggled, this, &Tab1Data::on_pPBServerConnect_toggled);
    connect(ui->pPBSend, &QPushButton::clicked, this, &Tab1Data::on_pPBSend_clicked);
    connect(ui->pPBMenu, &QPushButton::clicked, this, &Tab1Data::menuButtonClicked);
    connect(ui->pLErecvid, &QLineEdit::returnPressed, ui->pPBSend, &QPushButton::click);
    connect(ui->pLEsendData, &QLineEdit::returnPressed, ui->pPBSend, &QPushButton::click);

    // 터미널 초기화
    ui->pTErecvdata->setReadOnly(true);

    // DB
    ensureDb();
    setupTableHeader();
}

Tab1Data::~Tab1Data(){ delete ui; }

void Tab1Data::ensureDb()
{
    // 1) 우선순위 1: 환경변수로 강제 경로 지정 가능 (옵션)
    const QByteArray envDb = qgetenv("AIOT_DB");

    // 2) 기본: 사용자 데이터 폴더 (예: /home/pi/.local/share/AiotClient/)
    QString baseDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    if (baseDir.isEmpty()) baseDir = QDir::homePath() + "/.local/share/AiotClient";

    QDir().mkpath(baseDir);
    const QString dbPath = envDb.isEmpty() ? (baseDir + "/aiot.db")
                                           : QString::fromLocal8Bit(envDb);

    db = QSqlDatabase::addDatabase("QSQLITE", "events_conn");
    db.setDatabaseName(dbPath);

    if (!db.open()) {
        QMessageBox::warning(this, "DB open fail",
                             "Open error: " + db.lastError().text() + "\npath=" + dbPath);
        return;
    }

    // SQLite 동시접속/내구성 밸런스
    QSqlQuery q(db);
    q.exec("PRAGMA journal_mode=WAL");
    q.exec("PRAGMA synchronous=NORMAL");
    q.exec("PRAGMA busy_timeout=3000");

    // 테이블 생성
    const char* ddl =
        "CREATE TABLE IF NOT EXISTS events ("
        " id INTEGER PRIMARY KEY AUTOINCREMENT,"
        " detection_time TEXT NOT NULL,"
        " x REAL NOT NULL, y REAL NOT NULL, z REAL NOT NULL,"
        " raw TEXT)";
    if (!q.exec(ddl)) {
        QMessageBox::warning(this, "DB schema",
                             "DDL error: " + q.lastError().text());
    }

    // (선택) 경로 로그로 확인
    qDebug() << "[DB] using" << db.databaseName();
}


void Tab1Data::setupTableHeader()
{
    ui->pDBTable->setColumnCount(5);
    ui->pDBTable->setHorizontalHeaderLabels({"ID","Time","X","Y","Z"});
    ui->pDBTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->pDBTable->setSelectionBehavior(QAbstractItemView::SelectRows);
}

void Tab1Data::appendTerminal(const QString& line)
{
    ui->pTErecvdata->append(QTime::currentTime().toString("HH:mm:ss") + " " + line);
}

void Tab1Data::on_pPBServerConnect_toggled(bool checked)
{
    if (checked) {
        bool ok;
        // Get IP address from user
        QString ip = QInputDialog::getText(this, "Server IP", "Server IP:", QLineEdit::Normal, "192.168.0.172", &ok);

        if (ok && !ip.isEmpty()) {
            // User entered an IP and clicked OK
            sock->connectWithAddress(ip, 5000); // Use the new function with port 5000
            // The button text will be updated by onSocketConnected() slot
        } else {
            // User cancelled or entered nothing, so un-toggle the button
            ui->pPBServerConnect->setChecked(false);
        }
    } else {
        sock->socketClosedServerSlot();
        // The button text will be updated by onSocketDisconnected() slot
    }
}

void Tab1Data::onSocketConnected()    { appendTerminal("[INFO] Connected");  ui->pPBServerConnect->setText("Sever Disconnect"); ui->pPBServerConnect->setChecked(true); }
void Tab1Data::onSocketDisconnected() { appendTerminal("[INFO] Disconnected"); ui->pPBServerConnect->setText("Server Connect"); ui->pPBServerConnect->setChecked(false); }
void Tab1Data::onSocketError(QString msg){ appendTerminal("[ERROR] " + msg); }

void Tab1Data::onRecvLine(QString line)
{
    if (!line.isEmpty() && line.endsWith('\r')) line.chop(1);
    appendTerminal(line);
    tryIngestFromLine(line);
}

void Tab1Data::tryIngestFromLine(const QString& line)
{
    // 원문 다듬기
    QString s = line.trimmed();
    if (s.startsWith("[RASPBERRYPI]")) {
        s.remove(0, QString("[RASPBERRYPI]").size());
    }
    s = s.trimmed();

    // 기대 포맷: "x@y@z@drone"
    const QStringList parts = s.split('@');
    if (parts.size() != 4 || parts[3].trimmed() != "drone") {
        qDebug() << "[INGEST-SKIP]" << s;
        return;
    }

    bool okX=false, okY=false, okZ=false;
    const double x = parts[0].toDouble(&okX);
    const double y = parts[1].toDouble(&okY);
    const double z = parts[2].toDouble(&okZ);

    if (!(okX && okY && okZ)) {
        appendTerminal("[INGEST-ERR] parse fail: " + s);
        return;
    }

    insertEvent(x, y, z);
    emit droneDetected(x, y, z);

    // 디버그 출력 (둘 중 하나만)
    qDebug() << "[INGEST]" << x << y << z;
    // printf("%.3f %.3f %.3f\n", x, y, z); fflush(stdout);
}

void Tab1Data::insertEvent(double x, double y, double z)
{
    if (!db.isOpen()) { appendTerminal("[DB-ERR] db not open"); return; }

    const QString now = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
    const QString raw = QString("%1@%2@%3@drone").arg(x).arg(y).arg(z);

    QSqlQuery q(db);

    // 1) 이름 바인딩(가장 안전) - addBindValue()와 섞지 말 것
    q.prepare(
        "INSERT INTO events (detection_time, x, y, z, raw) "
        "VALUES (:ts, :x, :y, :z, :raw)"
        );
    q.bindValue(":ts",  now);
    q.bindValue(":x",   x);
    q.bindValue(":y",   y);
    q.bindValue(":z",   z);
    q.bindValue(":raw", raw);

    appendTerminal("[DBG] driver=" + db.driverName() + " sql=" + q.lastQuery() + " (named)");
    appendTerminal(QString("[DBG] binds=5 ts=%1 x=%2 y=%3 z=%4 raw=%5")
                       .arg(now).arg(x).arg(y).arg(z).arg(raw));

    if (!q.exec()) {
        const QString err = q.lastError().text();
        appendTerminal("[DB-ERR] " + err);

        // 2) 여전히 mismatch면, 임시 우회(Fallback): 즉시실행 SQL로 강제 삽입 (디버깅용)
        if (err.contains("Parameter count mismatch", Qt::CaseInsensitive)) {
            QString escRaw = raw; escRaw.replace('\'', "''"); // 작은따옴표 이스케이프
            QString sql = QString(
                              "INSERT INTO events (detection_time, x, y, z, raw) "
                              "VALUES ('%1', %2, %3, %4, '%5')"
                              ).arg(now)
                              .arg(QString::number(x, 'f', 6))
                              .arg(QString::number(y, 'f', 6))
                              .arg(QString::number(z, 'f', 6))
                              .arg(escRaw);

            appendTerminal("[DB-WARN] fallback SQL=" + sql);
            QSqlQuery q2(db);
            if (!q2.exec(sql)) {
                appendTerminal("[DB-ERR Fallback] " + q2.lastError().text());
            } else {
                appendTerminal("[DB-OK] fallback inserted");
            }
        }
    } else {
        appendTerminal("[DB-OK] row inserted");
    }
}




void Tab1Data::on_pPBQuery_clicked()
{
    loadSnapshotAndRender();
}

void Tab1Data::loadSnapshotAndRender()
{
    if (!db.isOpen()) return;
    const auto asOf = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");

    QSqlQuery q(db);
    q.prepare("SELECT id, detection_time, x, y, z "
              "FROM events WHERE detection_time <= :asof ORDER BY detection_time ASC");
    q.bindValue(":asof", asOf);
    if (!q.exec()) { appendTerminal("[DB-ERR] " + q.lastError().text()); return; }

    ui->pDBTable->setRowCount(0);

    int row=0;
    while (q.next()) {
        ui->pDBTable->insertRow(row);
        ui->pDBTable->setItem(row,0, new QTableWidgetItem(q.value(0).toString()));
        ui->pDBTable->setItem(row,1, new QTableWidgetItem(q.value(1).toString()));
        ui->pDBTable->setItem(row,2, new QTableWidgetItem(q.value(2).toString()));
        ui->pDBTable->setItem(row,3, new QTableWidgetItem(q.value(3).toString()));
        ui->pDBTable->setItem(row,4, new QTableWidgetItem(q.value(4).toString()));
        row++;
    }
    ui->pDBTable->resizeColumnsToContents();
}

void Tab1Data::on_pPBSend_clicked()
{
    if (!ui->pLEsendData || !ui->pLErecvid) { // Check both exist
        return;
    }

    QString payload = ui->pLEsendData->text();
    if (payload.isEmpty()) {
        return;
    }

    QString recvId = ui->pLErecvid->text().trimmed();

    const QString wire = recvId.isEmpty()
                             ? "[ALLMSG]" + payload
                             : "[" + recvId + "]" + payload;

    sock->socketWriteDataSlot(wire);

    ui->pLEsendData->clear();
}
