#ifndef MAINWIDGET_H
#define MAINWIDGET_H

#include <QWidget>

#include "tab1data.h"
#include "tab2camstream.h" // Uncommented

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

    Tab1Data *pTab1Data; // tab1
    Tab2CamStream *pTab2CamStream; // tab2 // Uncommented

};
#endif // MAINWIDGET_H
