/********************************************************************************
** Form generated from reading UI file 'tab1data.ui'
**
** Created by: Qt User Interface Compiler version 6.8.3
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

class Ui_pTabWidget
{
public:
    QVBoxLayout *verticalLayout;
    QHBoxLayout *horizontalLayout;
    QPushButton *pPBQuery;
    QPushButton *pPBServerConnect;
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

    void setupUi(QWidget *pTabWidget)
    {
        if (pTabWidget->objectName().isEmpty())
            pTabWidget->setObjectName("pTabWidget");
        pTabWidget->resize(661, 464);
        verticalLayout = new QVBoxLayout(pTabWidget);
        verticalLayout->setObjectName("verticalLayout");
        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName("horizontalLayout");
        pPBQuery = new QPushButton(pTabWidget);
        pPBQuery->setObjectName("pPBQuery");

        horizontalLayout->addWidget(pPBQuery);

        pPBServerConnect = new QPushButton(pTabWidget);
        pPBServerConnect->setObjectName("pPBServerConnect");
        pPBServerConnect->setCheckable(true);

        horizontalLayout->addWidget(pPBServerConnect);

        widget = new QWidget(pTabWidget);
        widget->setObjectName("widget");

        horizontalLayout->addWidget(widget);

        DBchart = new QLabel(pTabWidget);
        DBchart->setObjectName("DBchart");
        DBchart->setAlignment(Qt::AlignmentFlag::AlignRight|Qt::AlignmentFlag::AlignTrailing|Qt::AlignmentFlag::AlignVCenter);

        horizontalLayout->addWidget(DBchart);


        verticalLayout->addLayout(horizontalLayout);

        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setObjectName("horizontalLayout_2");
        pDBTable = new QTableWidget(pTabWidget);
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
        pDBTable->setObjectName("pDBTable");
        pDBTable->setRowCount(0);
        pDBTable->setColumnCount(5);
        pDBTable->horizontalHeader()->setDefaultSectionSize(57);

        horizontalLayout_2->addWidget(pDBTable);

        verticalLayout_2 = new QVBoxLayout();
        verticalLayout_2->setObjectName("verticalLayout_2");
        horizontalLayout_3 = new QHBoxLayout();
        horizontalLayout_3->setObjectName("horizontalLayout_3");
        pTErecvdata = new QTextEdit(pTabWidget);
        pTErecvdata->setObjectName("pTErecvdata");

        horizontalLayout_3->addWidget(pTErecvdata);


        verticalLayout_2->addLayout(horizontalLayout_3);

        pLErecvid = new QLineEdit(pTabWidget);
        pLErecvid->setObjectName("pLErecvid");

        verticalLayout_2->addWidget(pLErecvid);

        pLEsendData = new QLineEdit(pTabWidget);
        pLEsendData->setObjectName("pLEsendData");

        verticalLayout_2->addWidget(pLEsendData);

        pPBSend = new QPushButton(pTabWidget);
        pPBSend->setObjectName("pPBSend");

        verticalLayout_2->addWidget(pPBSend);

        verticalLayout_2->setStretch(0, 1);

        horizontalLayout_2->addLayout(verticalLayout_2);

        horizontalLayout_2->setStretch(0, 5);
        horizontalLayout_2->setStretch(1, 5);

        verticalLayout->addLayout(horizontalLayout_2);


        retranslateUi(pTabWidget);

        QMetaObject::connectSlotsByName(pTabWidget);
    } // setupUi

    void retranslateUi(QWidget *pTabWidget)
    {
        pTabWidget->setWindowTitle(QCoreApplication::translate("pTabWidget", "Form", nullptr));
        pPBQuery->setText(QCoreApplication::translate("pTabWidget", "\354\241\260\355\232\214", nullptr));
        pPBServerConnect->setText(QCoreApplication::translate("pTabWidget", "\354\204\234\353\262\204\354\227\260\352\262\260", nullptr));
        DBchart->setText(QCoreApplication::translate("pTabWidget", "\354\210\230\354\213\240\353\215\260\354\235\264\355\204\260", nullptr));
        QTableWidgetItem *___qtablewidgetitem = pDBTable->horizontalHeaderItem(0);
        ___qtablewidgetitem->setText(QCoreApplication::translate("pTabWidget", "ID", nullptr));
        QTableWidgetItem *___qtablewidgetitem1 = pDBTable->horizontalHeaderItem(1);
        ___qtablewidgetitem1->setText(QCoreApplication::translate("pTabWidget", "Detection_Time", nullptr));
        QTableWidgetItem *___qtablewidgetitem2 = pDBTable->horizontalHeaderItem(2);
        ___qtablewidgetitem2->setText(QCoreApplication::translate("pTabWidget", "Locate", nullptr));
        QTableWidgetItem *___qtablewidgetitem3 = pDBTable->horizontalHeaderItem(3);
        ___qtablewidgetitem3->setText(QCoreApplication::translate("pTabWidget", "Distance_Level", nullptr));
        pLErecvid->setPlaceholderText(QCoreApplication::translate("pTabWidget", "Recipient ID (optional)", nullptr));
        pPBSend->setText(QCoreApplication::translate("pTabWidget", "\354\206\241\354\213\240", nullptr));
    } // retranslateUi

};

namespace Ui {
    class pTabWidget: public Ui_pTabWidget {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_TAB1DATA_H
