/********************************************************************************
** Form generated from reading UI file 'tab1devcontrol.ui'
**
** Created by: Qt User Interface Compiler version 6.8.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_TAB1DEVCONTROL_H
#define UI_TAB1DEVCONTROL_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDial>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLCDNumber>
#include <QtWidgets/QProgressBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_Tab1DevControl
{
public:
    QHBoxLayout *horizontalLayout_5;
    QVBoxLayout *verticalLayout;
    QHBoxLayout *horizontalLayout_2;
    QPushButton *pPBtimerStart;
    QComboBox *pCBtimerValue;
    QPushButton *pPBquit;
    QHBoxLayout *horizontalLayout;
    QDial *pDialLed;
    QLCDNumber *pLcdNumberLed;
    QProgressBar *pProgressBarLed;
    QHBoxLayout *horizontalLayout_4;
    QGridLayout *gridLayout_5;
    QCheckBox *pCBkey8;
    QCheckBox *pCBkey7;
    QCheckBox *pCBkey4;
    QCheckBox *pCBkey2;
    QCheckBox *pCBkey6;
    QCheckBox *pCBkey5;
    QCheckBox *pCBkey3;
    QCheckBox *pCBkey1;
    QLCDNumber *pLcdNumberKey;
    QHBoxLayout *horizontalLayout_3;

    void setupUi(QWidget *Tab1DevControl)
    {
        if (Tab1DevControl->objectName().isEmpty())
            Tab1DevControl->setObjectName("Tab1DevControl");
        Tab1DevControl->resize(928, 740);
        Tab1DevControl->setMinimumSize(QSize(300, 350));
        horizontalLayout_5 = new QHBoxLayout(Tab1DevControl);
        horizontalLayout_5->setObjectName("horizontalLayout_5");
        verticalLayout = new QVBoxLayout();
        verticalLayout->setObjectName("verticalLayout");
        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setObjectName("horizontalLayout_2");
        pPBtimerStart = new QPushButton(Tab1DevControl);
        pPBtimerStart->setObjectName("pPBtimerStart");
        pPBtimerStart->setCheckable(true);

        horizontalLayout_2->addWidget(pPBtimerStart);

        pCBtimerValue = new QComboBox(Tab1DevControl);
        pCBtimerValue->addItem(QString());
        pCBtimerValue->addItem(QString());
        pCBtimerValue->addItem(QString());
        pCBtimerValue->addItem(QString());
        pCBtimerValue->addItem(QString());
        pCBtimerValue->addItem(QString());
        pCBtimerValue->setObjectName("pCBtimerValue");

        horizontalLayout_2->addWidget(pCBtimerValue);

        pPBquit = new QPushButton(Tab1DevControl);
        pPBquit->setObjectName("pPBquit");

        horizontalLayout_2->addWidget(pPBquit);

        horizontalLayout_2->setStretch(0, 2);
        horizontalLayout_2->setStretch(1, 2);
        horizontalLayout_2->setStretch(2, 1);

        verticalLayout->addLayout(horizontalLayout_2);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName("horizontalLayout");
        pDialLed = new QDial(Tab1DevControl);
        pDialLed->setObjectName("pDialLed");
        pDialLed->setMaximum(255);
        pDialLed->setWrapping(true);
        pDialLed->setNotchTarget(3.700000000000000);
        pDialLed->setNotchesVisible(true);

        horizontalLayout->addWidget(pDialLed);

        pLcdNumberLed = new QLCDNumber(Tab1DevControl);
        pLcdNumberLed->setObjectName("pLcdNumberLed");
        pLcdNumberLed->setSmallDecimalPoint(false);
        pLcdNumberLed->setDigitCount(2);
        pLcdNumberLed->setMode(QLCDNumber::Mode::Hex);
        pLcdNumberLed->setSegmentStyle(QLCDNumber::SegmentStyle::Filled);

        horizontalLayout->addWidget(pLcdNumberLed);


        verticalLayout->addLayout(horizontalLayout);

        pProgressBarLed = new QProgressBar(Tab1DevControl);
        pProgressBarLed->setObjectName("pProgressBarLed");
        pProgressBarLed->setMaximum(255);
        pProgressBarLed->setValue(0);
        pProgressBarLed->setAlignment(Qt::AlignmentFlag::AlignCenter);

        verticalLayout->addWidget(pProgressBarLed);

        horizontalLayout_4 = new QHBoxLayout();
        horizontalLayout_4->setObjectName("horizontalLayout_4");
        gridLayout_5 = new QGridLayout();
        gridLayout_5->setObjectName("gridLayout_5");
        pCBkey8 = new QCheckBox(Tab1DevControl);
        pCBkey8->setObjectName("pCBkey8");

        gridLayout_5->addWidget(pCBkey8, 0, 0, 1, 1);

        pCBkey7 = new QCheckBox(Tab1DevControl);
        pCBkey7->setObjectName("pCBkey7");

        gridLayout_5->addWidget(pCBkey7, 0, 1, 1, 1);

        pCBkey4 = new QCheckBox(Tab1DevControl);
        pCBkey4->setObjectName("pCBkey4");

        gridLayout_5->addWidget(pCBkey4, 1, 0, 1, 1);

        pCBkey2 = new QCheckBox(Tab1DevControl);
        pCBkey2->setObjectName("pCBkey2");

        gridLayout_5->addWidget(pCBkey2, 1, 2, 1, 1);

        pCBkey6 = new QCheckBox(Tab1DevControl);
        pCBkey6->setObjectName("pCBkey6");

        gridLayout_5->addWidget(pCBkey6, 0, 2, 1, 1);

        pCBkey5 = new QCheckBox(Tab1DevControl);
        pCBkey5->setObjectName("pCBkey5");

        gridLayout_5->addWidget(pCBkey5, 0, 3, 1, 1);

        pCBkey3 = new QCheckBox(Tab1DevControl);
        pCBkey3->setObjectName("pCBkey3");

        gridLayout_5->addWidget(pCBkey3, 1, 1, 1, 1);

        pCBkey1 = new QCheckBox(Tab1DevControl);
        pCBkey1->setObjectName("pCBkey1");

        gridLayout_5->addWidget(pCBkey1, 1, 3, 1, 1);


        horizontalLayout_4->addLayout(gridLayout_5);

        pLcdNumberKey = new QLCDNumber(Tab1DevControl);
        pLcdNumberKey->setObjectName("pLcdNumberKey");
        pLcdNumberKey->setDigitCount(2);
        pLcdNumberKey->setMode(QLCDNumber::Mode::Hex);

        horizontalLayout_4->addWidget(pLcdNumberKey);

        horizontalLayout_4->setStretch(0, 1);
        horizontalLayout_4->setStretch(1, 1);

        verticalLayout->addLayout(horizontalLayout_4);

        horizontalLayout_3 = new QHBoxLayout();
        horizontalLayout_3->setObjectName("horizontalLayout_3");

        verticalLayout->addLayout(horizontalLayout_3);

        verticalLayout->setStretch(0, 1);
        verticalLayout->setStretch(1, 4);
        verticalLayout->setStretch(2, 1);
        verticalLayout->setStretch(3, 4);

        horizontalLayout_5->addLayout(verticalLayout);


        retranslateUi(Tab1DevControl);
        QObject::connect(pDialLed, &QDial::valueChanged, pLcdNumberLed, qOverload<int>(&QLCDNumber::display));

        QMetaObject::connectSlotsByName(Tab1DevControl);
    } // setupUi

    void retranslateUi(QWidget *Tab1DevControl)
    {
        Tab1DevControl->setWindowTitle(QCoreApplication::translate("Tab1DevControl", "Form", nullptr));
        pPBtimerStart->setText(QCoreApplication::translate("Tab1DevControl", "TimerStart", nullptr));
        pCBtimerValue->setItemText(0, QCoreApplication::translate("Tab1DevControl", "10", nullptr));
        pCBtimerValue->setItemText(1, QCoreApplication::translate("Tab1DevControl", "50", nullptr));
        pCBtimerValue->setItemText(2, QCoreApplication::translate("Tab1DevControl", "100", nullptr));
        pCBtimerValue->setItemText(3, QCoreApplication::translate("Tab1DevControl", "500", nullptr));
        pCBtimerValue->setItemText(4, QCoreApplication::translate("Tab1DevControl", "1000", nullptr));
        pCBtimerValue->setItemText(5, QCoreApplication::translate("Tab1DevControl", "2000", nullptr));

        pPBquit->setText(QCoreApplication::translate("Tab1DevControl", "Quit", nullptr));
        pCBkey8->setText(QCoreApplication::translate("Tab1DevControl", "8", nullptr));
        pCBkey7->setText(QCoreApplication::translate("Tab1DevControl", "7", nullptr));
        pCBkey4->setText(QCoreApplication::translate("Tab1DevControl", "4", nullptr));
        pCBkey2->setText(QCoreApplication::translate("Tab1DevControl", "2", nullptr));
        pCBkey6->setText(QCoreApplication::translate("Tab1DevControl", "6", nullptr));
        pCBkey5->setText(QCoreApplication::translate("Tab1DevControl", "5", nullptr));
        pCBkey3->setText(QCoreApplication::translate("Tab1DevControl", "3", nullptr));
        pCBkey1->setText(QCoreApplication::translate("Tab1DevControl", "1", nullptr));
    } // retranslateUi

};

namespace Ui {
    class Tab1DevControl: public Ui_Tab1DevControl {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_TAB1DEVCONTROL_H
