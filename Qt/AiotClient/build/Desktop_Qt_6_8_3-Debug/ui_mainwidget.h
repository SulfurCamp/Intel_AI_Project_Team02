/********************************************************************************
** Form generated from reading UI file 'mainwidget.ui'
**
** Created by: Qt User Interface Compiler version 6.8.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWIDGET_H
#define UI_MAINWIDGET_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWidget
{
public:
    QVBoxLayout *verticalLayout;
    QTabWidget *pTabWidget;
    QWidget *pTab1;
    QWidget *pTab2;
    QWidget *pTab3;
    QWidget *pTab4;
    QWidget *pTab5;
    QWidget *pTab6;
    QWidget *pTab7;

    void setupUi(QWidget *MainWidget)
    {
        if (MainWidget->objectName().isEmpty())
            MainWidget->setObjectName("MainWidget");
        MainWidget->resize(650, 600);
        MainWidget->setMinimumSize(QSize(650, 600));
        verticalLayout = new QVBoxLayout(MainWidget);
        verticalLayout->setObjectName("verticalLayout");
        pTabWidget = new QTabWidget(MainWidget);
        pTabWidget->setObjectName("pTabWidget");
        pTabWidget->setDocumentMode(true);
        pTab1 = new QWidget();
        pTab1->setObjectName("pTab1");
        pTabWidget->addTab(pTab1, QString());
        pTab2 = new QWidget();
        pTab2->setObjectName("pTab2");
        pTabWidget->addTab(pTab2, QString());
        pTab3 = new QWidget();
        pTab3->setObjectName("pTab3");
        pTabWidget->addTab(pTab3, QString());
        pTab4 = new QWidget();
        pTab4->setObjectName("pTab4");
        pTabWidget->addTab(pTab4, QString());
        pTab5 = new QWidget();
        pTab5->setObjectName("pTab5");
        pTabWidget->addTab(pTab5, QString());
        pTab6 = new QWidget();
        pTab6->setObjectName("pTab6");
        pTabWidget->addTab(pTab6, QString());
        pTab7 = new QWidget();
        pTab7->setObjectName("pTab7");
        pTabWidget->addTab(pTab7, QString());

        verticalLayout->addWidget(pTabWidget);


        retranslateUi(MainWidget);

        pTabWidget->setCurrentIndex(6);


        QMetaObject::connectSlotsByName(MainWidget);
    } // setupUi

    void retranslateUi(QWidget *MainWidget)
    {
        MainWidget->setWindowTitle(QCoreApplication::translate("MainWidget", "MainWidget", nullptr));
        pTabWidget->setTabText(pTabWidget->indexOf(pTab1), QCoreApplication::translate("MainWidget", "Tab 1", nullptr));
        pTabWidget->setTabText(pTabWidget->indexOf(pTab2), QCoreApplication::translate("MainWidget", "Tab 2", nullptr));
        pTabWidget->setTabText(pTabWidget->indexOf(pTab3), QCoreApplication::translate("MainWidget", "Tab 3", nullptr));
        pTabWidget->setTabText(pTabWidget->indexOf(pTab4), QCoreApplication::translate("MainWidget", "Tab 4", nullptr));
        pTabWidget->setTabText(pTabWidget->indexOf(pTab5), QCoreApplication::translate("MainWidget", "Tab 5", nullptr));
        pTabWidget->setTabText(pTabWidget->indexOf(pTab6), QCoreApplication::translate("MainWidget", "Tab 6", nullptr));
        pTabWidget->setTabText(pTabWidget->indexOf(pTab7), QCoreApplication::translate("MainWidget", "Tab 7", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWidget: public Ui_MainWidget {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWIDGET_H
