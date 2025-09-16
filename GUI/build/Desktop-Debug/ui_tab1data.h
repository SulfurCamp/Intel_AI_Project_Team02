/********************************************************************************
** Form generated from reading UI file 'tab1data.ui'
**
** Created by: Qt User Interface Compiler version 5.15.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_TAB1DATA_H
#define UI_TAB1DATA_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QTableWidget>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_Tab1Data
{
public:
    QVBoxLayout *verticalLayout;
    QHBoxLayout *horizontalLayout;
    QPushButton *pPBQuery;
    QPushButton *pPBServerConnect;
    QPushButton *pPBMenu;
    QWidget *widget;
    QLabel *DBchart;
    QHBoxLayout *horizontalLayout_2;
    QTableWidget *pDBTable;
    QVBoxLayout *verticalLayout_2;
    QHBoxLayout *horizontalLayout_3;
    QTextEdit *pTErecvdata;
    QLineEdit *pLErecvid;
    QLineEdit *pLEsendData;
    QPushButton *pPBSend;

    void setupUi(QWidget *Tab1Data)
    {
        if (Tab1Data->objectName().isEmpty())
            Tab1Data->setObjectName(QString::fromUtf8("Tab1Data"));
        Tab1Data->resize(661, 464);
        verticalLayout = new QVBoxLayout(Tab1Data);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        pPBQuery = new QPushButton(Tab1Data);
        pPBQuery->setObjectName(QString::fromUtf8("pPBQuery"));

        horizontalLayout->addWidget(pPBQuery);

        pPBServerConnect = new QPushButton(Tab1Data);
        pPBServerConnect->setObjectName(QString::fromUtf8("pPBServerConnect"));
        pPBServerConnect->setCheckable(true);

        horizontalLayout->addWidget(pPBServerConnect);

        pPBMenu = new QPushButton(Tab1Data);
        pPBMenu->setObjectName(QString::fromUtf8("pPBMenu"));

        horizontalLayout->addWidget(pPBMenu);

        widget = new QWidget(Tab1Data);
        widget->setObjectName(QString::fromUtf8("widget"));

        horizontalLayout->addWidget(widget);

        DBchart = new QLabel(Tab1Data);
        DBchart->setObjectName(QString::fromUtf8("DBchart"));
        DBchart->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout->addWidget(DBchart);


        verticalLayout->addLayout(horizontalLayout);

        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        pDBTable = new QTableWidget(Tab1Data);
        if (pDBTable->columnCount() < 5)
            pDBTable->setColumnCount(5);
        QTableWidgetItem *__qtablewidgetitem = new QTableWidgetItem();
        pDBTable->setHorizontalHeaderItem(0, __qtablewidgetitem);
        QTableWidgetItem *__qtablewidgetitem1 = new QTableWidgetItem();
        pDBTable->setHorizontalHeaderItem(1, __qtablewidgetitem1);
        QTableWidgetItem *__qtablewidgetitem2 = new QTableWidgetItem();
        pDBTable->setHorizontalHeaderItem(2, __qtablewidgetitem2);
        QTableWidgetItem *__qtablewidgetitem3 = new QTableWidgetItem();
        pDBTable->setHorizontalHeaderItem(3, __qtablewidgetitem3);
        pDBTable->setObjectName(QString::fromUtf8("pDBTable"));
        pDBTable->setRowCount(0);
        pDBTable->setColumnCount(5);
        pDBTable->horizontalHeader()->setDefaultSectionSize(57);

        horizontalLayout_2->addWidget(pDBTable);

        verticalLayout_2 = new QVBoxLayout();
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        horizontalLayout_3 = new QHBoxLayout();
        horizontalLayout_3->setObjectName(QString::fromUtf8("horizontalLayout_3"));
        pTErecvdata = new QTextEdit(Tab1Data);
        pTErecvdata->setObjectName(QString::fromUtf8("pTErecvdata"));

        horizontalLayout_3->addWidget(pTErecvdata);


        verticalLayout_2->addLayout(horizontalLayout_3);

        pLErecvid = new QLineEdit(Tab1Data);
        pLErecvid->setObjectName(QString::fromUtf8("pLErecvid"));

        verticalLayout_2->addWidget(pLErecvid);

        pLEsendData = new QLineEdit(Tab1Data);
        pLEsendData->setObjectName(QString::fromUtf8("pLEsendData"));

        verticalLayout_2->addWidget(pLEsendData);

        pPBSend = new QPushButton(Tab1Data);
        pPBSend->setObjectName(QString::fromUtf8("pPBSend"));

        verticalLayout_2->addWidget(pPBSend);

        verticalLayout_2->setStretch(0, 1);

        horizontalLayout_2->addLayout(verticalLayout_2);

        horizontalLayout_2->setStretch(0, 5);
        horizontalLayout_2->setStretch(1, 5);

        verticalLayout->addLayout(horizontalLayout_2);


        retranslateUi(Tab1Data);

        QMetaObject::connectSlotsByName(Tab1Data);
    } // setupUi

    void retranslateUi(QWidget *Tab1Data)
    {
        Tab1Data->setWindowTitle(QCoreApplication::translate("Tab1Data", "Form", nullptr));
        pPBQuery->setText(QCoreApplication::translate("Tab1Data", "Query", nullptr));
        pPBServerConnect->setText(QCoreApplication::translate("Tab1Data", "Connect to Server", nullptr));
        pPBMenu->setText(QCoreApplication::translate("Tab1Data", "Back to Menu", nullptr));
        DBchart->setText(QCoreApplication::translate("Tab1Data", "Received Data", nullptr));
        QTableWidgetItem *___qtablewidgetitem = pDBTable->horizontalHeaderItem(0);
        ___qtablewidgetitem->setText(QCoreApplication::translate("Tab1Data", "ID", nullptr));
        QTableWidgetItem *___qtablewidgetitem1 = pDBTable->horizontalHeaderItem(1);
        ___qtablewidgetitem1->setText(QCoreApplication::translate("Tab1Data", "Detection_Time", nullptr));
        QTableWidgetItem *___qtablewidgetitem2 = pDBTable->horizontalHeaderItem(2);
        ___qtablewidgetitem2->setText(QCoreApplication::translate("Tab1Data", "Locate", nullptr));
        QTableWidgetItem *___qtablewidgetitem3 = pDBTable->horizontalHeaderItem(3);
        ___qtablewidgetitem3->setText(QCoreApplication::translate("Tab1Data", "Distance_Level", nullptr));
        pLErecvid->setPlaceholderText(QCoreApplication::translate("Tab1Data", "Recipient ID (optional)", nullptr));
        pPBSend->setText(QCoreApplication::translate("Tab1Data", "Send", nullptr));
    } // retranslateUi

};

namespace Ui {
    class Tab1Data: public Ui_Tab1Data {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_TAB1DATA_H
