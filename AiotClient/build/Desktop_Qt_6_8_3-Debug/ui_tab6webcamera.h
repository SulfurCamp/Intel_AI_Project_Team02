/********************************************************************************
** Form generated from reading UI file 'tab6webcamera.ui'
**
** Created by: Qt User Interface Compiler version 6.8.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_TAB6WEBCAMERA_H
#define UI_TAB6WEBCAMERA_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QGraphicsView>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_Tab6WebCamera
{
public:
    QVBoxLayout *verticalLayout_2;
    QVBoxLayout *verticalLayout;
    QGraphicsView *pGPView;
    QHBoxLayout *horizontalLayout;
    QPushButton *pPBCamStart;
    QPushButton *pPBSnapshot;

    void setupUi(QWidget *Tab6WebCamera)
    {
        if (Tab6WebCamera->objectName().isEmpty())
            Tab6WebCamera->setObjectName("Tab6WebCamera");
        Tab6WebCamera->resize(438, 375);
        verticalLayout_2 = new QVBoxLayout(Tab6WebCamera);
        verticalLayout_2->setObjectName("verticalLayout_2");
        verticalLayout = new QVBoxLayout();
        verticalLayout->setObjectName("verticalLayout");
        pGPView = new QGraphicsView(Tab6WebCamera);
        pGPView->setObjectName("pGPView");

        verticalLayout->addWidget(pGPView);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName("horizontalLayout");
        pPBCamStart = new QPushButton(Tab6WebCamera);
        pPBCamStart->setObjectName("pPBCamStart");
        pPBCamStart->setCheckable(true);

        horizontalLayout->addWidget(pPBCamStart);

        pPBSnapshot = new QPushButton(Tab6WebCamera);
        pPBSnapshot->setObjectName("pPBSnapshot");

        horizontalLayout->addWidget(pPBSnapshot);


        verticalLayout->addLayout(horizontalLayout);

        verticalLayout->setStretch(0, 9);
        verticalLayout->setStretch(1, 1);

        verticalLayout_2->addLayout(verticalLayout);


        retranslateUi(Tab6WebCamera);

        QMetaObject::connectSlotsByName(Tab6WebCamera);
    } // setupUi

    void retranslateUi(QWidget *Tab6WebCamera)
    {
        Tab6WebCamera->setWindowTitle(QCoreApplication::translate("Tab6WebCamera", "Form", nullptr));
        pPBCamStart->setText(QCoreApplication::translate("Tab6WebCamera", "CamStart", nullptr));
        pPBSnapshot->setText(QCoreApplication::translate("Tab6WebCamera", "Snapshot", nullptr));
    } // retranslateUi

};

namespace Ui {
    class Tab6WebCamera: public Ui_Tab6WebCamera {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_TAB6WEBCAMERA_H
