#include "tab2camstream.h"
#include "ui_tab2camstream.h"

#include <QMediaDevices>
#include <QMessageBox>
#include <QVideoFrame>
#include <QBuffer>
#include <QFile>
#include <QDateTime>

Tab2CamStream::Tab2CamStream(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Tab2CamStream)
{
    ui->setupUi(this);

    // Assign the promoted widget directly
    videoWidget = qobject_cast<QVideoWidget*>(ui->pGPView);

    // Setup camera
    QList<QCameraDevice> cameras = QMediaDevices::videoInputs();
    if (cameras.isEmpty()) {
        QMessageBox::critical(this, "Camera Error", "No camera found!");
        ui->pPBCamStart->setEnabled(false);
        ui->pPBSnapshot->setEnabled(false);
        return;
    }

    camera = new QCamera(cameras.first());
    captureSession = new QMediaCaptureSession(this);
    captureSession->setCamera(camera);
    captureSession->setVideoOutput(videoWidget);

    // Setup image capture
    imageCapture = new QImageCapture(captureSession);
    captureSession->setImageCapture(imageCapture);

    // Connect signals for image capture
    connect(imageCapture, &QImageCapture::imageCaptured, this, &Tab2CamStream::imageCaptured);
    connect(imageCapture, &QImageCapture::errorOccurred, this, [this](int id, const QImageCapture::Error error, const QString &errorString){
        QMessageBox::critical(this, "Image Capture Error", errorString);
    });

    // Connect buttons
    connect(ui->pPBCamStart, &QPushButton::toggled, this, &Tab2CamStream::on_pPBCamStart_toggled);
    connect(ui->pPBSnapshot, &QPushButton::clicked, this, &Tab2CamStream::on_pPBSnapshot_clicked);

    ui->pPBSnapshot->setEnabled(false);
}

Tab2CamStream::~Tab2CamStream()
{
    if (camera && camera->isActive()) {
        camera->stop();
    }
    delete ui;
    delete camera;
    delete captureSession;
    delete imageCapture;
}

void Tab2CamStream::on_pPBCamStart_toggled(bool checked)
{
    if (checked) {
        camera->start();
        ui->pPBCamStart->setText("CamStop");
        ui->pPBSnapshot->setEnabled(true);
    } else {
        camera->stop();
        ui->pPBCamStart->setText("CamStart");
        ui->pPBSnapshot->setEnabled(false);
    }
}

void Tab2CamStream::on_pPBSnapshot_clicked()
{
    if (camera && camera->isActive()) {
        imageCapture->capture();
    }
}

void Tab2CamStream::imageCaptured(int id, const QImage &preview)
{
    // Save the captured image to a file
    QString fileName = QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss") + ".jpg";
    if (preview.save(fileName, "JPG")) {
        QMessageBox::information(this, "Snapshot", "Image saved as " + fileName);
    } else {
        QMessageBox::critical(this, "Snapshot Error", "Failed to save image.");
    }
}