#include "tab3controlpannel.h"
#include "ui_tab3controlpannel.h"

Tab3ControlPannel::Tab3ControlPannel(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Tab3ControlPannel)
{
    ui->setupUi(this);

    paletteOn.setColor(ui->pPBlamp->backgroundRole(), QColor(255, 0, 0)); // r, g, b
    paletteOff.setColor(ui->pPBlamp->backgroundRole(), QColor(0, 255, 0));
    ui->pPBlamp->setPalette(paletteOff);
    ui->pPBplug->setPalette(paletteOff);
}

Tab3ControlPannel::~Tab3ControlPannel()
{
    delete ui;
}

void Tab3ControlPannel::on_pPBlamp_clicked(bool checked)
{
    if(checked)
    {
        emit socketSendDataSig("[KMS_LIN]LAMPON");
        ui->pPBlamp->setChecked(false); // off일때 off유지
    }
    else
    {
        emit socketSendDataSig("[KMS_LIN]LAMPOFF");
        ui->pPBlamp->setChecked(true); // on일때 on유지
    }
}

void Tab3ControlPannel::tab3RecvDataSlot(QString recvData)
{
    qDebug() << recvData;
    QStringList strList = recvData.split("@");
    if(strList[2] == "LAMPON") // 클라이언트가 LAMPON을 보내면 버튼 ON으로 변경
    {
        ui->pPBlamp->setChecked(true);
        ui->pPBlamp->setPalette(paletteOn);
    }
    else if(strList[2] == "LAMPOFF") // 클라이언트가 LAMPOFF를 보내면 버튼 OFF로 변경
    {
        ui->pPBlamp->setChecked(false);
        ui->pPBlamp->setPalette(paletteOff);
    }
    else if(strList[2] == "PLUGON")
    {
        ui->pPBplug->setChecked(true);
        ui->pPBplug->setPalette(paletteOn);
    }
    else if(strList[2] == "PLUGOFF")
    {
        ui->pPBplug->setChecked(false);
        ui->pPBplug->setPalette(paletteOff);
    }
}

void Tab3ControlPannel::on_pPBplug_clicked(bool checked)
{
    if(checked)
    {
        emit socketSendDataSig("[KMS_LIN]PLUGON");
        ui->pPBplug->setChecked(false); // off일때 off유지
    }
    else
    {
        emit socketSendDataSig("[KMS_LIN]PLUGOFF");
        ui->pPBplug->setChecked(true); // on일때 on유지
    }
}

