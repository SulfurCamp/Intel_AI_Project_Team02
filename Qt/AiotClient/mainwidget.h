#ifndef MAINWIDGET_H
#define MAINWIDGET_H

#include <QWidget>

#include "tab1devcontrol.h"
#include "tab2socketclient.h"
#include "tab3controlpannel.h"
#include "tab4sensorchart.h"
#include "tab5sensordatabase.h"
#include "tab6webcamera.h"
#include "tab7camviewerthread.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWidget;
}
QT_END_NAMESPACE

class MainWidget : public QWidget
{
    Q_OBJECT

public:
    MainWidget(QWidget *parent = nullptr);
    ~MainWidget();

private:
    Ui::MainWidget *ui;

    Tab1DevControl *pTab1DevControl; // tab1
    Tab2socketclient *pTab2socketclient; // tab2
    Tab3ControlPannel *pTab3ControlPannel; // tab3
    Tab4SensorChart *pTab4SensorChart; // tab4
    Tab5sensordatabase *pTab5sensordatabase; // tab5
    Tab6WebCamera *pTab6WebCamera; // tab6
    Tab7CamViewerThread * pTab7CamViewerThread; // tab7

    LedKeyDev *keyvalue;
};
#endif // MAINWIDGET_H
