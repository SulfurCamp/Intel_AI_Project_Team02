#ifndef TAB3CONTROLPANNEL_H
#define TAB3CONTROLPANNEL_H

#include <QWidget>
#include <QPalette>
#include <QDebug>

namespace Ui {
class Tab3ControlPannel;
}

class Tab3ControlPannel : public QWidget
{
    Q_OBJECT

public:
    explicit Tab3ControlPannel(QWidget *parent = nullptr);
    ~Tab3ControlPannel();

signals:
    void socketSendDataSig(QString);

private slots:
    void on_pPBlamp_clicked(bool checked);
    void tab3RecvDataSlot(QString);

    void on_pPBplug_clicked(bool checked);

private:
    Ui::Tab3ControlPannel *ui;
    QPalette paletteOn;
    QPalette paletteOff;
};

#endif // TAB3CONTROLPANNEL_H
