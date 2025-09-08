/********************************************************************************
** Form generated from reading UI file 'tab7camviewerthread.ui'
**
** Created by: Qt User Interface Compiler version 6.8.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_TAB7CAMVIEWERTHREAD_H
#define UI_TAB7CAMVIEWERTHREAD_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_Tab7CamViewerThread
{
public:
    QVBoxLayout *verticalLayout_2;
    QVBoxLayout *verticalLayout;
    QLabel *plabelCamView;
    QHBoxLayout *horizontalLayout;
    QPushButton *pPBcamStart;
    QCheckBox *pCbRgb;
    QPushButton *pPBsnapShot;

    void setupUi(QWidget *Tab7CamViewerThread)
    {
        if (Tab7CamViewerThread->objectName().isEmpty())
            Tab7CamViewerThread->setObjectName("Tab7CamViewerThread");
        Tab7CamViewerThread->resize(515, 436);
        verticalLayout_2 = new QVBoxLayout(Tab7CamViewerThread);
        verticalLayout_2->setObjectName("verticalLayout_2");
        verticalLayout = new QVBoxLayout();
        verticalLayout->setObjectName("verticalLayout");
        plabelCamView = new QLabel(Tab7CamViewerThread);
        plabelCamView->setObjectName("plabelCamView");
        plabelCamView->setPixmap(QPixmap(QString::fromUtf8(":/Images/Images/initDisplay.png")));

        verticalLayout->addWidget(plabelCamView);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName("horizontalLayout");
        pPBcamStart = new QPushButton(Tab7CamViewerThread);
        pPBcamStart->setObjectName("pPBcamStart");
        pPBcamStart->setCheckable(true);

        horizontalLayout->addWidget(pPBcamStart);

        pCbRgb = new QCheckBox(Tab7CamViewerThread);
        pCbRgb->setObjectName("pCbRgb");

        horizontalLayout->addWidget(pCbRgb);

        pPBsnapShot = new QPushButton(Tab7CamViewerThread);
        pPBsnapShot->setObjectName("pPBsnapShot");

        horizontalLayout->addWidget(pPBsnapShot);

        horizontalLayout->setStretch(0, 4);
        horizontalLayout->setStretch(1, 2);
        horizontalLayout->setStretch(2, 4);

        verticalLayout->addLayout(horizontalLayout);

        verticalLayout->setStretch(0, 9);
        verticalLayout->setStretch(1, 1);

        verticalLayout_2->addLayout(verticalLayout);


        retranslateUi(Tab7CamViewerThread);

        QMetaObject::connectSlotsByName(Tab7CamViewerThread);
    } // setupUi

    void retranslateUi(QWidget *Tab7CamViewerThread)
    {
        Tab7CamViewerThread->setWindowTitle(QCoreApplication::translate("Tab7CamViewerThread", "Form", nullptr));
        plabelCamView->setText(QString());
        pPBcamStart->setText(QCoreApplication::translate("Tab7CamViewerThread", "CamStart", nullptr));
        pCbRgb->setText(QCoreApplication::translate("Tab7CamViewerThread", "RGB Classfy", nullptr));
        pPBsnapShot->setText(QCoreApplication::translate("Tab7CamViewerThread", "Snapshot", nullptr));
    } // retranslateUi

};

namespace Ui {
    class Tab7CamViewerThread: public Ui_Tab7CamViewerThread {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_TAB7CAMVIEWERTHREAD_H
