#ifndef MJPEGCLIENT_H
#define MJPEGCLIENT_H

#include <QObject>
#include <QImage>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QUrl>

class MjpegClient : public QObject {
    Q_OBJECT
public:
    explicit MjpegClient(const QUrl& url, QObject* parent = nullptr);
    void start();
    void stop();

signals:
    void frameReady(const QImage& img);

private slots:
    void onReadyRead();
    void onFinished();
    void onError(QNetworkReply::NetworkError code);

private:
    QUrl url_;
    QNetworkAccessManager nam_;
    QNetworkReply* reply_ = nullptr;
    QByteArray buffer_;
};

#endif // MJPEGCLIENT_H
