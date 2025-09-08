/********************************************************************************
** Form generated from reading UI file 'tab2camstream.ui'
**
** Created by: Qt User Interface Compiler version 6.8.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_TAB2CAMSTREAM_H
#define UI_TAB2CAMSTREAM_H

#include <QtCore/QVariant>
#include <QtMultimediaWidgets/QVideoWidget>
#include <QtWidgets/QApplication>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_Tab2CamStream
{
public:
    QVBoxLayout *verticalLayout_2;
    QVBoxLayout *verticalLayout;
    QVideoWidget *pGPView;
    QHBoxLayout *horizontalLayout;
    QPushButton *pPBCamStart;
    QPushButton *pPBSnapshot;

    void setupUi(QWidget *Tab2CamStream)
    {
        if (Tab2CamStream->objectName().isEmpty())
            Tab2CamStream->setObjectName("Tab2CamStream");
        Tab2CamStream->resize(438, 375);
        verticalLayout_2 = new QVBoxLayout(Tab2CamStream);
        verticalLayout_2->setObjectName("verticalLayout_2");
        verticalLayout = new QVBoxLayout();
        verticalLayout->setObjectName("verticalLayout");
        pGPView = new QVideoWidget(Tab2CamStream);
        pGPView->setObjectName("pGPView");

        verticalLayout->addWidget(pGPView);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName("horizontalLayout");
        pPBCamStart = new QPushButton(Tab2CamStream);
        pPBCamStart->setObjectName("pPBCamStart");
        pPBCamStart->setCheckable(true);

        horizontalLayout->addWidget(pPBCamStart);

        pPBSnapshot = new QPushButton(Tab2CamStream);
        pPBSnapshot->setObjectName("pPBSnapshot");

        horizontalLayout->addWidget(pPBSnapshot);


        verticalLayout->addLayout(horizontalLayout);

        verticalLayout->setStretch(0, 9);
        verticalLayout->setStretch(1, 1);

        verticalLayout_2->addLayout(verticalLayout);


        retranslateUi(Tab2CamStream);

        QMetaObject::connectSlotsByName(Tab2CamStream);
    } // setupUi

    void retranslateUi(QWidget *Tab2CamStream)
    {
        Tab2CamStream->setWindowTitle(QCoreApplication::translate("Tab2CamStream", "Form", nullptr));
        pPBCamStart->setText(QCoreApplication::translate("Tab2CamStream", "CamStart", nullptr));
        pPBSnapshot->setText(QCoreApplication::translate("Tab2CamStream", "Snapshot", nullptr));
    } // retranslateUi

};

namespace Ui {
    class Tab2CamStream: public Ui_Tab2CamStream {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_TAB2CAMSTREAM_H
