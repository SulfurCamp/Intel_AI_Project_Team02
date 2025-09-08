#include "tab1devcontrol.h"
#include "ui_tab1devcontrol.h"

Tab1DevControl::Tab1DevControl(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Tab1DevControl)
{
    ui->setupUi(this);
    int keyCount = ui->gridLayout_5->rowCount() * ui->gridLayout_5->columnCount();
    // qDebug() << "key" << keyCount;
    lcdData = 0;
    pLedKeyDev = new LedKeyDev(this);
    pQTimer = new QTimer(this);
    pQButtonGroup = new QButtonGroup(this);

    for (int i = 0; i < ui->gridLayout_5->rowCount(); i++)
    {
        for(int j = 0; j < ui->gridLayout_5->columnCount(); j++)
        {
            pQCheckBox[--keyCount] = dynamic_cast<QCheckBox *>(ui->gridLayout_5->itemAtPosition(i, j)->widget());
            // qDebug() << "i :" << i;
            // qDebug() << "j :" << j;
        }
    }

    pQButtonGroup->setExclusive(false);

    keyCount = ui->gridLayout_5->rowCount() * ui->gridLayout_5->columnCount();
    for(int i = 0; i < keyCount; i++)
        pQButtonGroup->addButton(pQCheckBox[i], i + 1);

    // connect(pQButtonGroup, SIGNAL(buttonClicked(int)), this, SLOT(updateCheckBoxMouseSlot(int)));

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    connect(pQButtonGroup, SIGNAL(idClicked(int)), this, SLOT(updateCheckBoxMouseSlot(int)));
#else
    connect(pQButtonGroup, SIGNAL(buttonClicked(int)), this, SLOT(updateCheckBoxMouseSlot(int)));
#endif

    connect(pQTimer,SIGNAL(timeout()), this, SLOT(updateDialValueSlot()));
    connect(ui->pDialLed, SIGNAL(valueChanged(int)), pLedKeyDev, SLOT(writeLedDataSlot(int)));
//    connect(ui->pDialLed, SIGNAL(valueChanged(int)), ui->pProgressBarLed,SLOT(setValue(int)));
    connect(ui->pDialLed, SIGNAL(valueChanged(int)), this,SLOT(updateProgressBarLedSlot(int)));
    connect(ui->pPBquit, SIGNAL(clicked()), qApp, SLOT(quit()));
    connect(pLedKeyDev, SIGNAL(updateKeyDataSig(int)), this, SLOT(updateCheckBoxKeySlot(int)));
}

void Tab1DevControl::updateProgressBarLedSlot(int value)
{
    ui->pProgressBarLed->setValue(value);
}
Tab1DevControl::~Tab1DevControl()
{
    delete ui;
}

void Tab1DevControl::on_pPBtimerStart_clicked(bool checked)
{
    if(checked)
    {
        QString strValue = ui->pCBtimerValue->currentText();
        pQTimer->start(strValue.toInt());
        ui->pPBtimerStart->setText("TimerStop");
    }
    else
    {
        pQTimer->stop();
        ui->pPBtimerStart->setText("TimerStart");
    }
    // qDebug() << "checked : " << checked;
}

void Tab1DevControl::updateDialValueSlot()
{
    int dialValue = ui->pDialLed->value();
    dialValue++;
    if(dialValue > ui->pDialLed->maximum())
        dialValue = 0;
    ui->pDialLed->setValue(dialValue);
}


void Tab1DevControl::on_pCBtimerValue_currentTextChanged(const QString &arg1)
{
    if(pQTimer->isActive())
    {
        pQTimer->stop();
        pQTimer->start(arg1.toInt());
    }
}

void Tab1DevControl::updateCheckBoxKeySlot(int keyNo)
{
    // QCheckBox *pQCheckBox[8] = {ui->pCBkey1, ui->pCBkey2, ui->pCBkey3, ui->pCBkey4, ui->pCBkey5, ui->pCBkey6, ui->pCBkey7, ui->pCBkey8};
    // static unsigned char lcdData = 0;
    lcdData = lcdData ^ (0x01 << (keyNo - 1));
    ui->pLcdNumberKey->display(lcdData);

    pLedKeyDev->writeLedDataSlot(lcdData);

    for (int i = 0; i < 8; i++)
    {
        if(ui->pPBtimerStart->isChecked())
        {
            // 라즈베리파이에 버튼을 클릭하면 다이얼과 lcd 타이머를 초기화
            ui->pPBtimerStart->setChecked(false);
            on_pPBtimerStart_clicked(false);
            // ui->pDialLed->setValue(0);
        }
        if (keyNo == i+1)
        {
            if(pQCheckBox[i]->isChecked())
                pQCheckBox[i]->setChecked(false);
            else
                pQCheckBox[i]->setChecked(true);
        }
    }
    ui->pDialLed->setValue(lcdData); // 버튼 값으로 lcd를 초기화
}

void Tab1DevControl::updateCheckBoxMouseSlot(int keyNo)
{
    lcdData = lcdData ^ (0x01 << (keyNo - 1));
    ui->pLcdNumberKey->display(lcdData);

    pLedKeyDev->writeLedDataSlot(lcdData);

    if(ui->pPBtimerStart->isChecked())
    {
        // 체크박스를 클릭하면 다이얼과 lcd 타이머를 초기화
        ui->pPBtimerStart->setChecked(false);
        on_pPBtimerStart_clicked(false);
        // ui->pDialLed->setValue(0);
    }
    ui->pDialLed->setValue(lcdData); // 버튼 값으로 lcd를 초기화
}

QDial* Tab1DevControl::getpDial() // 객체 포인터를 리턴
{
    return ui->pDialLed;
}

LedKeyDev* Tab1DevControl::getpLedKeyDev()
{
    return pLedKeyDev;
}
