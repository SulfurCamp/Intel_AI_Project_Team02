#include "tab1data.h"
#include "ui_tab1data.h"

#include <QMessageBox>
#include <QRegularExpression>
#include <QTableWidgetItem>

Tab1Data::Tab1Data(QWidget* parent)
    : QWidget(parent), ui(new Ui::pTabWidget)
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
    // SQLite 파일: aiot_events.db
    db = QSqlDatabase::addDatabase("QSQLITE", "events_conn");
    db.setDatabaseName("aiot.db");
    if (!db.open()) {
        QMessageBox::warning(this, "DB", "DB open fail: " + db.lastError().text());
        return;
    }
    QSqlQuery q(db);
    // ID/날짜/위치/거리
    q.exec("CREATE TABLE IF NOT EXISTS events ("
           "id INTEGER PRIMARY KEY AUTOINCREMENT,"
           "detection_time TEXT NOT NULL,"       /*yyyy-MM-dd HH:mm:ss*/
           "locate TEXT NOT NULL,"
           "distance_level INTEGER NOT NULL,"
           "raw TEXT)");
}

void Tab1Data::setupTableHeader()
{
    ui->pDBTable->setColumnCount(4);
    ui->pDBTable->setHorizontalHeaderLabels({"ID","Time","Locate","Distance_Level"});
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
        bool dummy=true; // 기존 connectToServerSlot(bool&) 시그니처 맞추려면 유지
        sock->connectToServerSlot(dummy);
        if (!dummy) ui->pPBServerConnect->setChecked(false);
        else ui->pPBServerConnect->setText("서버 해제");
    } else {
        sock->socketClosedServerSlot();
        ui->pPBServerConnect->setText("서버 연결");
    }
}

void Tab1Data::onSocketConnected()    { appendTerminal("[INFO] Connected");  ui->pPBServerConnect->setText("서버 해제"); ui->pPBServerConnect->setChecked(true); }
void Tab1Data::onSocketDisconnected() { appendTerminal("[INFO] Disconnected"); ui->pPBServerConnect->setText("서버 연결"); ui->pPBServerConnect->setChecked(false); }
void Tab1Data::onSocketError(QString msg){ appendTerminal("[ERROR] " + msg); }

void Tab1Data::onRecvLine(QString line)
{
    // 라인 단위 수신 (SockClient에서 \n 스플릿)
    if (!line.isEmpty() && line.endsWith('\r')) line.chop(1);
    appendTerminal(line);
    tryIngestFromLine(line);  // "A@1" → DB 적재
}

void Tab1Data::tryIngestFromLine(const QString& line)
{
    // Check if the line is from RASPBERRYPI
    if (!line.startsWith("[RASPBERRYPI]")) {
        return;
    }

    // Extract payload, e.g., "Q@1" from "[RASPBERRYPI]Q@1"
    QString payload = line.mid(13); // Length of "[RASPBERRYPI]" is 13

    // Use regex to parse "L@D" format from the payload.
    // L must be one of [QWEASDZXC]
    // D must be one of [123]
    QRegularExpression re("^([QWEASDZXC])@([1-3])$");
    auto match = re.match(payload.trimmed()); // Use trimmed() to remove leading/trailing whitespace

    if (match.hasMatch()) {
        QString locate = match.captured(1); // e.g., "Q"
        int dist = match.captured(2).toInt(); // e.g., 1
        insertEvent(locate, dist);
    }
}

void Tab1Data::insertEvent(const QString& locate, int distance)
{
    if (!db.isOpen()) return;
    QSqlQuery q(db);
    q.prepare("INSERT INTO events(detection_time, locate, distance_level, raw)"
              " VALUES(:ts, :loc, :dist, :raw)");
    auto now = QDateTime::currentDateTime();
    q.bindValue(":ts",   now.toString("yyyy-MM-dd HH:mm:ss"));
    q.bindValue(":loc",  locate);
    q.bindValue(":dist", distance);
    q.bindValue(":raw",  locate + "@" + QString::number(distance));
    if (!q.exec()) {
        appendTerminal("[DB-ERR] " + q.lastError().text());
    }
}

void Tab1Data::on_pPBQuery_clicked()
{
    loadSnapshotAndRender(); // 클릭 시점까지 스냅샷
}

void Tab1Data::loadSnapshotAndRender()
{
    if (!db.isOpen()) return;
    const auto asOf = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");

    QSqlQuery q(db);
    q.prepare("SELECT id, detection_time, locate, distance_level "
              "FROM events WHERE detection_time <= :asof ORDER BY detection_time ASC");
    q.bindValue(":asof", asOf);
    if (!q.exec()) { appendTerminal("[DB-ERR] " + q.lastError().text()); return; }

    // 테이블 초기화
    ui->pDBTable->setRowCount(0);

    int row=0;
    while (q.next()) {
        ui->pDBTable->insertRow(row);
        ui->pDBTable->setItem(row,0, new QTableWidgetItem(q.value(0).toString()));
        ui->pDBTable->setItem(row,1, new QTableWidgetItem(q.value(1).toString()));
        ui->pDBTable->setItem(row,2, new QTableWidgetItem(q.value(2).toString()));
        ui->pDBTable->setItem(row,3, new QTableWidgetItem(q.value(3).toString()));
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