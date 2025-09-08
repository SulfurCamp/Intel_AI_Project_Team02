/********************************************************************************
** Form generated from reading UI file 'tab5sensordatabase.ui'
**
** Created by: Qt User Interface Compiler version 6.8.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_TAB5SENSORDATABASE_H
#define UI_TAB5SENSORDATABASE_H

#include <QtCore/QDate>
#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDateTimeEdit>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QTableWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_Tab5sensordatabase
{
public:
    QVBoxLayout *verticalLayout;
    QHBoxLayout *horizontalLayout;
    QDateTimeEdit *pDateTimeEditFrom;
    QDateTimeEdit *pDateTimeEditTo;
    QPushButton *pPBsearchDB;
    QPushButton *pPBdeleteDB;
    QHBoxLayout *horizontalLayout_2;
    QTableWidget *pTBsensor;
    QVBoxLayout *vertical;
    QHBoxLayout *horizontalLayout_3;
    QSpacerItem *horizontalSpacer;
    QPushButton *pushButton;
    QHBoxLayout *pChartViewLayout;

    void setupUi(QWidget *Tab5sensordatabase)
    {
        if (Tab5sensordatabase->objectName().isEmpty())
            Tab5sensordatabase->setObjectName("Tab5sensordatabase");
        Tab5sensordatabase->resize(661, 464);
        verticalLayout = new QVBoxLayout(Tab5sensordatabase);
        verticalLayout->setObjectName("verticalLayout");
        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName("horizontalLayout");
        pDateTimeEditFrom = new QDateTimeEdit(Tab5sensordatabase);
        pDateTimeEditFrom->setObjectName("pDateTimeEditFrom");
        pDateTimeEditFrom->setDate(QDate(2025, 7, 16));
        pDateTimeEditFrom->setTime(QTime(9, 0, 0));

        horizontalLayout->addWidget(pDateTimeEditFrom);

        pDateTimeEditTo = new QDateTimeEdit(Tab5sensordatabase);
        pDateTimeEditTo->setObjectName("pDateTimeEditTo");
        pDateTimeEditTo->setDate(QDate(2025, 12, 31));

        horizontalLayout->addWidget(pDateTimeEditTo);

        pPBsearchDB = new QPushButton(Tab5sensordatabase);
        pPBsearchDB->setObjectName("pPBsearchDB");

        horizontalLayout->addWidget(pPBsearchDB);

        pPBdeleteDB = new QPushButton(Tab5sensordatabase);
        pPBdeleteDB->setObjectName("pPBdeleteDB");

        horizontalLayout->addWidget(pPBdeleteDB);


        verticalLayout->addLayout(horizontalLayout);

        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setObjectName("horizontalLayout_2");
        pTBsensor = new QTableWidget(Tab5sensordatabase);
        if (pTBsensor->columnCount() < 5)
            pTBsensor->setColumnCount(5);
        QTableWidgetItem *__qtablewidgetitem = new QTableWidgetItem();
        pTBsensor->setHorizontalHeaderItem(0, __qtablewidgetitem);
        QTableWidgetItem *__qtablewidgetitem1 = new QTableWidgetItem();
        pTBsensor->setHorizontalHeaderItem(1, __qtablewidgetitem1);
        QTableWidgetItem *__qtablewidgetitem2 = new QTableWidgetItem();
        pTBsensor->setHorizontalHeaderItem(2, __qtablewidgetitem2);
        QTableWidgetItem *__qtablewidgetitem3 = new QTableWidgetItem();
        pTBsensor->setHorizontalHeaderItem(3, __qtablewidgetitem3);
        QTableWidgetItem *__qtablewidgetitem4 = new QTableWidgetItem();
        pTBsensor->setHorizontalHeaderItem(4, __qtablewidgetitem4);
        pTBsensor->setObjectName("pTBsensor");
        pTBsensor->horizontalHeader()->setDefaultSectionSize(50);

        horizontalLayout_2->addWidget(pTBsensor);

        vertical = new QVBoxLayout();
        vertical->setObjectName("vertical");
        horizontalLayout_3 = new QHBoxLayout();
        horizontalLayout_3->setObjectName("horizontalLayout_3");
        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        horizontalLayout_3->addItem(horizontalSpacer);

        pushButton = new QPushButton(Tab5sensordatabase);
        pushButton->setObjectName("pushButton");

        horizontalLayout_3->addWidget(pushButton);

        horizontalLayout_3->setStretch(0, 6);
        horizontalLayout_3->setStretch(1, 4);

        vertical->addLayout(horizontalLayout_3);

        pChartViewLayout = new QHBoxLayout();
        pChartViewLayout->setObjectName("pChartViewLayout");

        vertical->addLayout(pChartViewLayout);

        vertical->setStretch(0, 1);
        vertical->setStretch(1, 9);

        horizontalLayout_2->addLayout(vertical);

        horizontalLayout_2->setStretch(0, 5);
        horizontalLayout_2->setStretch(1, 5);

        verticalLayout->addLayout(horizontalLayout_2);


        retranslateUi(Tab5sensordatabase);

        QMetaObject::connectSlotsByName(Tab5sensordatabase);
    } // setupUi

    void retranslateUi(QWidget *Tab5sensordatabase)
    {
        Tab5sensordatabase->setWindowTitle(QCoreApplication::translate("Tab5sensordatabase", "Form", nullptr));
        pPBsearchDB->setText(QCoreApplication::translate("Tab5sensordatabase", "\354\241\260\355\232\214", nullptr));
        pPBdeleteDB->setText(QCoreApplication::translate("Tab5sensordatabase", "\354\202\255\354\240\234", nullptr));
        QTableWidgetItem *___qtablewidgetitem = pTBsensor->horizontalHeaderItem(0);
        ___qtablewidgetitem->setText(QCoreApplication::translate("Tab5sensordatabase", "ID", nullptr));
        QTableWidgetItem *___qtablewidgetitem1 = pTBsensor->horizontalHeaderItem(1);
        ___qtablewidgetitem1->setText(QCoreApplication::translate("Tab5sensordatabase", "\353\202\240\354\247\234", nullptr));
        QTableWidgetItem *___qtablewidgetitem2 = pTBsensor->horizontalHeaderItem(2);
        ___qtablewidgetitem2->setText(QCoreApplication::translate("Tab5sensordatabase", "\354\241\260\353\217\204", nullptr));
        QTableWidgetItem *___qtablewidgetitem3 = pTBsensor->horizontalHeaderItem(3);
        ___qtablewidgetitem3->setText(QCoreApplication::translate("Tab5sensordatabase", "\354\230\250\353\217\204", nullptr));
        QTableWidgetItem *___qtablewidgetitem4 = pTBsensor->horizontalHeaderItem(4);
        ___qtablewidgetitem4->setText(QCoreApplication::translate("Tab5sensordatabase", "\354\212\265\353\217\204", nullptr));
        pushButton->setText(QCoreApplication::translate("Tab5sensordatabase", "clear", nullptr));
    } // retranslateUi

};

namespace Ui {
    class Tab5sensordatabase: public Ui_Tab5sensordatabase {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_TAB5SENSORDATABASE_H
