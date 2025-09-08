#ifndef LEDKEYDEV_H
#define LEDKEYDEV_H

#include <QWidget>
#include <QFile>
#include <QMessageBox>
#include <QSocketNotifier>
#include <QDebug>

class LedKeyDev : public QWidget
{
    Q_OBJECT
private:
    QFile *pQFile;
    QSocketNotifier *pQSocketNotifier;
    QString DEVFILENAME = "/dev/ledkey";
public:
    explicit LedKeyDev(QWidget *parent = nullptr);

signals:
    void updateKeyDataSig(int);

private slots:
    void readKeyDataSlot(int);
public slots:
    void writeLedDataSlot(int);

};

#endif // LEDKEYDEV_H
