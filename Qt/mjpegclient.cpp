#include "mjpegclient.h"
#include <QTimer>
#include <QDebug>

MjpegClient::MjpegClient(const QUrl& url, QObject* parent)
    : QObject(parent), url_(url) {}

void MjpegClient::start() {
    qDebug() << "MjpegClient::start() called";
    if (reply_) {
        reply_->abort();
        reply_->deleteLater();
        reply_ = nullptr;
    }
    QNetworkRequest req(url_);
    reply_ = nam_.get(req);
    connect(reply_, &QNetworkReply::readyRead, this, &MjpegClient::onReadyRead);
    connect(reply_, &QNetworkReply::finished, this, &MjpegClient::onFinished);
    connect(reply_, &QNetworkReply::errorOccurred, this, &MjpegClient::onError);
}

void MjpegClient::stop() {
    qDebug() << "MjpegClient::stop() called";
    if (reply_) {
        disconnect(reply_, nullptr, this, nullptr);
        reply_->abort();
        reply_->deleteLater();
        reply_ = nullptr;
    }
}

void MjpegClient::onReadyRead() {
    qDebug() << "MjpegClient::onReadyRead() called";
    buffer_ += reply_->readAll();

    while (true) {
        int soi = buffer_.indexOf("\xFF\xD8", 0);
        if (soi < 0) {
            if (buffer_.size() > 1024 * 1024)
                buffer_ = buffer_.right(1024 * 512);
            break;
        }
        int eoi = buffer_.indexOf("\xFF\xD9", soi + 2);
        if (eoi < 0) {
            if (soi > 0 && soi > 1024 * 512)
                buffer_ = buffer_.mid(soi);
            break;
        }

        QByteArray jpeg = buffer_.mid(soi, eoi - soi + 2);
        buffer_ = buffer_.mid(eoi + 2);

        QImage img;
        img.loadFromData(jpeg, "JPG");
        if (!img.isNull()) emit frameReady(img);
    }
}

void MjpegClient::onFinished() {
    qDebug() << "MjpegClient::onFinished() called";
    // Reconnection logic can be implemented here if needed
    // For example, restart after a delay:
    // QTimer::singleShot(1000, this, [this]{ start(); });
}

void MjpegClient::onError(QNetworkReply::NetworkError code) {
    qWarning("MjpegClient::onError: Network error: %d", static_cast<int>(code));
    // Retry logic can be added here if needed
}