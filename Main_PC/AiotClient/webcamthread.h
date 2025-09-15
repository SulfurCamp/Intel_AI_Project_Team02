#ifndef WEBCAMTHREAD_H
#define WEBCAMTHREAD_H

#include <QThread>
#include <QLabel>
#include <QTimer>
#include <opencv2/opencv.hpp>

using namespace cv;
using namespace std;
class WebCamThread : public QThread
{
    Q_OBJECT
    void run();
    int cnt;
    bool rgbClassifyFlag;
    string fname;
    Mat frame, frameQt;
    QString strColor, strColorPre;
    QImage qImage;
    QTimer *pQTimer;
    void put_string(Mat &frame, string text, Point pt, int value = -1);

private slots:
    void rgbClassifySlot();

signals:
    void socketSendDataSig(QString);

public:
    WebCamThread(QObject *parent = nullptr);
    bool camViewFlag;
    QLabel *pCamView;
    void snapShot();
    void rgbTimerStart();
    void rgbTimerStop();
};

#endif // WEBCAMTHREAD_H
