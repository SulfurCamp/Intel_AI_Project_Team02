/********************************************************************************
** Form generated from reading UI file 'tab2camstream.ui'
**
** Created by: Qt User Interface Compiler version 5.15.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_TAB2CAMSTREAM_H
#define UI_TAB2CAMSTREAM_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_Tab2CamStream
{
public:
    QVBoxLayout *verticalLayout_2;
    QVBoxLayout *verticalLayout;
    QLabel *pGPView;
    QHBoxLayout *horizontalLayout;
    QPushButton *pPBCamStart;
    QPushButton *pPBSnapshot;
    QPushButton *pPBMenu;

    void setupUi(QWidget *Tab2CamStream)
    {
        if (Tab2CamStream->objectName().isEmpty())
            Tab2CamStream->setObjectName(QString::fromUtf8("Tab2CamStream"));
        Tab2CamStream->resize(438, 375);
        verticalLayout_2 = new QVBoxLayout(Tab2CamStream);
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        verticalLayout = new QVBoxLayout();
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        pGPView = new QLabel(Tab2CamStream);
        pGPView->setObjectName(QString::fromUtf8("pGPView"));

        verticalLayout->addWidget(pGPView);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        pPBCamStart = new QPushButton(Tab2CamStream);
        pPBCamStart->setObjectName(QString::fromUtf8("pPBCamStart"));
        pPBCamStart->setCheckable(true);

        horizontalLayout->addWidget(pPBCamStart);

        pPBSnapshot = new QPushButton(Tab2CamStream);
        pPBSnapshot->setObjectName(QString::fromUtf8("pPBSnapshot"));

        horizontalLayout->addWidget(pPBSnapshot);

        pPBMenu = new QPushButton(Tab2CamStream);
        pPBMenu->setObjectName(QString::fromUtf8("pPBMenu"));

        horizontalLayout->addWidget(pPBMenu);


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
        pPBMenu->setText(QCoreApplication::translate("Tab2CamStream", "\353\251\224\353\211\264\353\241\234 \353\217\214\354\225\204\352\260\200\352\270\260", nullptr));
    } // retranslateUi

};

namespace Ui {
    class Tab2CamStream: public Ui_Tab2CamStream {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_TAB2CAMSTREAM_H
