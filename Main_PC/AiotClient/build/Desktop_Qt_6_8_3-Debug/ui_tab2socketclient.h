/********************************************************************************
** Form generated from reading UI file 'tab2socketclient.ui'
**
** Created by: Qt User Interface Compiler version 6.8.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_TAB2SOCKETCLIENT_H
#define UI_TAB2SOCKETCLIENT_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_Tab2socketclient
{
public:
    QVBoxLayout *verticalLayout_4;
    QVBoxLayout *verticalLayout_3;
    QHBoxLayout *horizontalLayout;
    QLabel *label;
    QPushButton *pPBrecvDataClear;
    QPushButton *pPBserverConnect;
    QTextEdit *pTErecvdata;
    QHBoxLayout *horizontalLayout_2;
    QLineEdit *pLErecvid;
    QLineEdit *pLEsendData;
    QPushButton *pPBSend;

    void setupUi(QWidget *Tab2socketclient)
    {
        if (Tab2socketclient->objectName().isEmpty())
            Tab2socketclient->setObjectName("Tab2socketclient");
        Tab2socketclient->resize(400, 300);
        verticalLayout_4 = new QVBoxLayout(Tab2socketclient);
        verticalLayout_4->setObjectName("verticalLayout_4");
        verticalLayout_3 = new QVBoxLayout();
        verticalLayout_3->setObjectName("verticalLayout_3");
        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName("horizontalLayout");
        label = new QLabel(Tab2socketclient);
        label->setObjectName("label");

        horizontalLayout->addWidget(label);

        pPBrecvDataClear = new QPushButton(Tab2socketclient);
        pPBrecvDataClear->setObjectName("pPBrecvDataClear");

        horizontalLayout->addWidget(pPBrecvDataClear);

        pPBserverConnect = new QPushButton(Tab2socketclient);
        pPBserverConnect->setObjectName("pPBserverConnect");
        pPBserverConnect->setCheckable(true);

        horizontalLayout->addWidget(pPBserverConnect);

        horizontalLayout->setStretch(0, 6);
        horizontalLayout->setStretch(1, 2);

        verticalLayout_3->addLayout(horizontalLayout);

        pTErecvdata = new QTextEdit(Tab2socketclient);
        pTErecvdata->setObjectName("pTErecvdata");

        verticalLayout_3->addWidget(pTErecvdata);

        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setObjectName("horizontalLayout_2");
        pLErecvid = new QLineEdit(Tab2socketclient);
        pLErecvid->setObjectName("pLErecvid");

        horizontalLayout_2->addWidget(pLErecvid);

        pLEsendData = new QLineEdit(Tab2socketclient);
        pLEsendData->setObjectName("pLEsendData");

        horizontalLayout_2->addWidget(pLEsendData);

        pPBSend = new QPushButton(Tab2socketclient);
        pPBSend->setObjectName("pPBSend");

        horizontalLayout_2->addWidget(pPBSend);

        horizontalLayout_2->setStretch(0, 3);
        horizontalLayout_2->setStretch(1, 6);
        horizontalLayout_2->setStretch(2, 1);

        verticalLayout_3->addLayout(horizontalLayout_2);


        verticalLayout_4->addLayout(verticalLayout_3);


        retranslateUi(Tab2socketclient);
        QObject::connect(pLEsendData, &QLineEdit::returnPressed, pPBSend, qOverload<>(&QPushButton::click));

        QMetaObject::connectSlotsByName(Tab2socketclient);
    } // setupUi

    void retranslateUi(QWidget *Tab2socketclient)
    {
        Tab2socketclient->setWindowTitle(QCoreApplication::translate("Tab2socketclient", "Form", nullptr));
        label->setText(QCoreApplication::translate("Tab2socketclient", "\354\210\230\354\213\240 \353\215\260\354\235\264\355\204\260", nullptr));
        pPBrecvDataClear->setText(QCoreApplication::translate("Tab2socketclient", "\354\210\230\354\213\240 \354\202\255\354\240\234", nullptr));
        pPBserverConnect->setText(QCoreApplication::translate("Tab2socketclient", "\354\204\234\353\262\204 \354\227\260\352\262\260", nullptr));
        pPBSend->setText(QCoreApplication::translate("Tab2socketclient", "\354\206\241\354\213\240", nullptr));
    } // retranslateUi

};

namespace Ui {
    class Tab2socketclient: public Ui_Tab2socketclient {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_TAB2SOCKETCLIENT_H
