#ifndef TAB7CAMVIEWERTHREAD_H
#define TAB7CAMVIEWERTHREAD_H

#include <QWidget>
#include <QDebug>

#include "webcamthread.h"

namespace Ui {
class Tab7CamViewerThread;
}

class Tab7CamViewerThread : public QWidget
{
    Q_OBJECT

public:
    explicit Tab7CamViewerThread(QWidget *parent = nullptr);
    ~Tab7CamViewerThread();
    WebCamThread *getWebCamThread();

private slots:
    void on_pPBcamStart_clicked(bool checked);

    void on_pPBsnapShot_clicked();

    void on_pCbRgb_clicked(bool checked);

private:
    Ui::Tab7CamViewerThread *ui;

    WebCamThread *pWebCamThread;
};

#endif // TAB7CAMVIEWERTHREAD_H
