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

        verticalLayout->addWidget(pTabWidget);


        retranslateUi(MainWidget);

        pTabWidget->setCurrentIndex(0);


        QMetaObject::connectSlotsByName(MainWidget);
    } // setupUi

    void retranslateUi(QWidget *MainWidget)
    {
        MainWidget->setWindowTitle(QCoreApplication::translate("MainWidget", "MainWidget", nullptr));
        pTabWidget->setTabText(pTabWidget->indexOf(pTab1), QCoreApplication::translate("MainWidget", "\353\215\260\354\235\264\355\204\260", nullptr));
        pTabWidget->setTabText(pTabWidget->indexOf(pTab2), QCoreApplication::translate("MainWidget", "\354\271\264\353\251\224\353\235\274", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWidget: public Ui_MainWidget {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWIDGET_H
