#include "webcamthread.h"

WebCamThread::WebCamThread(QObject *parent)
    : QThread(parent)
{
    cnt = 0;
    camViewFlag = false;
    rgbClassifyFlag = false;
    strColor = "NONE";
    strColorPre = "NONE";
    pQTimer = new QTimer(this);
    connect(pQTimer, SIGNAL(timeout()), this, SLOT(rgbClassifySlot()));
}
void WebCamThread::run()
{
    VideoCapture  capture(0);
    if (!capture.isOpened())
    {
        cout << "카메라가 연결되지 않았습니다." << endl;
        exit(1);
    }
    while(camViewFlag) {

        capture.read(frame);

        // put_string(frame, "Count: ", Point(10, 40), cnt);
        fname = "cam_" + to_string(cnt++);
        fname += ".jpg";
        cvtColor(frame, frameQt, COLOR_BGR2RGB);
        // Size s = frameQt.size();
        int x = frameQt.cols / 2;
        int y = frameQt.rows / 2;

        if(rgbClassifyFlag)
        {
            Scalar meanHsv;
            Mat hsvImage, frameRoi;
            frameRoi = frame(Rect((x - 32), (y - 32), 64, 64));
            cvtColor(frameRoi, hsvImage, COLOR_BGR2HSV);
            meanHsv = mean(hsvImage);

            /* qDebug() << " meanHSV H : " << meanHsv[0] <<
               " meanHSV S : " << meanHsv[1] <<
                " meanHSV V : " << meanHsv[2]; // 색상, 채도, 명도 값 출력
            */
            if(170 <= meanHsv[0] || meanHsv[0] < 10) // Red
                strColor = "RED";
            else if(50 <= meanHsv[0] && meanHsv[0] < 70) // Green
                strColor = "GREEN";
            else if(110 <= meanHsv[0] && meanHsv[0] < 130) // Blue
                strColor = "BLUE";
            else
                strColor = "NONE";

            rgbClassifyFlag = false;
            if(strColor != strColorPre) // 컬러 색상 값이 바뀌었을때만
            {
                emit socketSendDataSig("[KMS_LIN]COLOR@" + strColor);
                strColorPre = strColor;
            }
        }
        // qDebug() << strColor.toStdString();
        put_string(frameQt, strColor.toStdString(), Point(10, 40));
        line(frameQt, Point((x - 32), y), Point((x + 32), y), Scalar(255, 0, 0), 2);
        line(frameQt, Point(x, (y - 32)), Point(x, (y + 32)), Scalar(255, 0, 0), 2);
        rectangle(frameQt, Point((x - 32), (y - 32)), Point((x + 32), (y + 32)), Scalar(0, 255, 0), 2);
        // QImage qImage(frame.data, frame.cols, frame.rows, QImage::Format_BGR888);
        qImage = QImage(frameQt.data, frameQt.cols, frameQt.rows, QImage::Format_RGB888);
        pCamView->setPixmap(QPixmap::fromImage(qImage));
    }
    capture.release();
    pCamView->setPixmap(QPixmap(":/Images/Images/initDisplay.png"));
}

// 문자열 출력 함수 - 그림자 효과
void WebCamThread::put_string(Mat &frame, string text, Point pt, int value)
{
    Scalar colorScalar;

    if(value != -1)
        text += to_string(value);

    if(text == "RED")
        colorScalar = {255, 0, 0};
    else if(text == "GREEN")
        colorScalar = {0, 255, 0};
    else if(text == "BLUE")
        colorScalar = {0, 0, 255};
    else // NONE
        colorScalar = {128, 128, 128};
    Point shade = pt + Point(2, 2);
    int font = FONT_HERSHEY_SIMPLEX;
    putText(frame, text, shade, font, 0.7, Scalar(0, 0, 0), 2);     // 그림자 효과
    putText(frame, text, pt, font, 0.7, colorScalar, 2);// 작성 문자
}
void WebCamThread::snapShot()
{
    // imwrite(fname,frame);
    // qDebug() << "SnapShot";
    qImage.save(QString::fromStdString(fname), "JPG", 80);
}

void WebCamThread::rgbTimerStart()
{
    pQTimer->start(1000); // 1초 : 1000
}

void WebCamThread::rgbTimerStop()
{
    if(pQTimer->isActive()) // 타이머가 구동중이면
        pQTimer->stop();
}

void WebCamThread::rgbClassifySlot()
{
    rgbClassifyFlag = true;
}
