#ifndef TAB2CAMSTREAM_H
#define TAB2CAMSTREAM_H

#include <QWidget>
#include <QGraphicsView>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>

// Qt Multimedia includes
#include <QCamera>
#include <QMediaCaptureSession>
#include <QVideoWidget>
#include <QImageCapture>
#include <QMediaDevices>

namespace Ui {
class Tab2CamStream;
}

class Tab2CamStream : public QWidget
{
    Q_OBJECT

public:
    explicit Tab2CamStream(QWidget *parent = nullptr);
    ~Tab2CamStream();

private:
    Ui::Tab2CamStream *ui;

    QCamera *camera;
    QMediaCaptureSession *captureSession;
    QVideoWidget *videoWidget;
    QImageCapture *imageCapture;

private slots:
    void on_pPBCamStart_toggled(bool checked);
    void on_pPBSnapshot_clicked();
    void imageCaptured(int id, const QImage &preview);
};

#endif // TAB2CAMSTREAM_H