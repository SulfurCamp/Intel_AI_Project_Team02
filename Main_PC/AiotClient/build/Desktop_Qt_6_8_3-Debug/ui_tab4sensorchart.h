/********************************************************************************
** Form generated from reading UI file 'tab4sensorchart.ui'
**
** Created by: Qt User Interface Compiler version 6.8.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_TAB4SENSORCHART_H
#define UI_TAB4SENSORCHART_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_Tab4SensorChart
{
public:
    QVBoxLayout *verticalLayout_2;
    QHBoxLayout *horizontalLayout;
    QSpacerItem *horizontalSpacer;
    QPushButton *pPBClearChart;
    QVBoxLayout *pChartViewLayout;

    void setupUi(QWidget *Tab4SensorChart)
    {
        if (Tab4SensorChart->objectName().isEmpty())
            Tab4SensorChart->setObjectName("Tab4SensorChart");
        Tab4SensorChart->resize(400, 300);
        verticalLayout_2 = new QVBoxLayout(Tab4SensorChart);
        verticalLayout_2->setObjectName("verticalLayout_2");
        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName("horizontalLayout");
        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        horizontalLayout->addItem(horizontalSpacer);

        pPBClearChart = new QPushButton(Tab4SensorChart);
        pPBClearChart->setObjectName("pPBClearChart");

        horizontalLayout->addWidget(pPBClearChart);

        horizontalLayout->setStretch(0, 8);
        horizontalLayout->setStretch(1, 2);

        verticalLayout_2->addLayout(horizontalLayout);

        pChartViewLayout = new QVBoxLayout();
        pChartViewLayout->setObjectName("pChartViewLayout");

        verticalLayout_2->addLayout(pChartViewLayout);

        verticalLayout_2->setStretch(0, 1);
        verticalLayout_2->setStretch(1, 9);

        retranslateUi(Tab4SensorChart);

        QMetaObject::connectSlotsByName(Tab4SensorChart);
    } // setupUi

    void retranslateUi(QWidget *Tab4SensorChart)
    {
        Tab4SensorChart->setWindowTitle(QCoreApplication::translate("Tab4SensorChart", "Form", nullptr));
        pPBClearChart->setText(QCoreApplication::translate("Tab4SensorChart", "Clear", nullptr));
    } // retranslateUi

};

namespace Ui {
    class Tab4SensorChart: public Ui_Tab4SensorChart {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_TAB4SENSORCHART_H
