#include "tab7camviewerthread.h"
#include "ui_tab7camviewerthread.h"

Tab7CamViewerThread::Tab7CamViewerThread(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Tab7CamViewerThread)
{
    ui->setupUi(this);

    ui->pPBsnapShot->setEnabled(false);
    ui->pCbRgb->setEnabled(false);
    pWebCamThread = new WebCamThread(this);
    pWebCamThread->pCamView = ui->plabelCamView;
}

Tab7CamViewerThread::~Tab7CamViewerThread()
{
    delete ui;
}

void Tab7CamViewerThread::on_pPBcamStart_clicked(bool checked)
{
    if(checked)
    {
        pWebCamThread->camViewFlag = true;
        // qDebug() << "on_pPBcamStart_clicked 1 ";
        if(!pWebCamThread->isRunning())
        {
            pWebCamThread->start();
            ui->pPBcamStart->setText("CamStop");
            ui->pPBsnapShot->setEnabled(true);
        }
    }
    else
    {
        // qDebug() << "on_pPBcamStart_clicked 2";
        pWebCamThread->camViewFlag = false;
        ui->pPBcamStart->setText("CamStart");
        ui->pPBsnapShot->setEnabled(false);
    }
    ui->pCbRgb->setEnabled(checked);
}

void Tab7CamViewerThread::on_pPBsnapShot_clicked()
{
    pWebCamThread->snapShot();
}


void Tab7CamViewerThread::on_pCbRgb_clicked(bool checked)
{
    if(checked)
    {
        pWebCamThread->rgbTimerStart();
    }
    else
    {
        pWebCamThread->rgbTimerStop();
    }
}

WebCamThread * Tab7CamViewerThread::getWebCamThread()
{
    return pWebCamThread;
}

