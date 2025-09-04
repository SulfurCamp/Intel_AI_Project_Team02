#ifndef TAB1DEVCONTROL_H
#define TAB1DEVCONTROL_H

#include <QWidget>
#include <QTimer>
#include <QButtonGroup>
#include <QDial>

#include "ledkeydev.h"

namespace Ui {
class Tab1DevControl;
}

class Tab1DevControl : public QWidget
{
    Q_OBJECT

public:
    explicit Tab1DevControl(QWidget *parent = nullptr);
    ~Tab1DevControl();
    QDial *getpDial();
    LedKeyDev *getpLedKeyDev();

private slots:
    void updateProgressBarLedSlot(int);
    void on_pPBtimerStart_clicked(bool checked);
    void updateDialValueSlot();

    void on_pCBtimerValue_currentTextChanged(const QString &arg1);

    void updateCheckBoxKeySlot(int);
    void updateCheckBoxMouseSlot(int);

private:
    Ui::Tab1DevControl *ui;
    LedKeyDev *pLedKeyDev;
    QTimer *pQTimer;
    QButtonGroup *pQButtonGroup;
    QCheckBox *pQCheckBox[8];
    unsigned char lcdData;
};

#endif // TAB1DEVCONTROL_H
