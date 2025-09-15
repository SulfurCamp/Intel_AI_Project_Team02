/********************************************************************************
** Form generated from reading UI file 'keyboard.ui'
**
** Created by: Qt User Interface Compiler version 6.8.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_KEYBOARD_H
#define UI_KEYBOARD_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_Keyboard
{
public:
    QVBoxLayout *verticalLayout;
    QHBoxLayout *horizontalLayout;
    QLineEdit *lineEdit;
    QPushButton *backButton;
    QPushButton *clear;
    QGridLayout *gridLayout;
    QPushButton *Buttonq;
    QPushButton *Buttonw;
    QPushButton *Buttone;
    QPushButton *Buttonr;
    QPushButton *Buttont;
    QPushButton *Buttony;
    QPushButton *Buttonu;
    QPushButton *Buttoni;
    QPushButton *Buttono;
    QPushButton *Buttons;
    QPushButton *Buttond;
    QPushButton *Buttonf;
    QPushButton *Buttong;
    QPushButton *Buttonh;
    QPushButton *Buttonj;
    QPushButton *Buttonl;
    QPushButton *Buttonz;
    QPushButton *Buttonx;
    QPushButton *Buttonv;
    QPushButton *Buttonb;
    QPushButton *Buttonn;
    QPushButton *Buttonm;
    QPushButton *char_2;
    QPushButton *space;
    QPushButton *Button0;
    QPushButton *Button1;
    QPushButton *Button2;
    QPushButton *Button3;
    QPushButton *Button4;
    QPushButton *Button5;
    QPushButton *Button6;
    QPushButton *Button7;
    QPushButton *Button8;
    QPushButton *Button9;
    QPushButton *Buttonp;
    QPushButton *enterButton;
    QPushButton *Buttona;
    QPushButton *shift;
    QPushButton *Buttonc;
    QPushButton *Buttonk;

    void setupUi(QWidget *Keyboard)
    {
        if (Keyboard->objectName().isEmpty())
            Keyboard->setObjectName("Keyboard");
        Keyboard->setWindowModality(Qt::NonModal);
        Keyboard->resize(600, 315);
        QSizePolicy sizePolicy(QSizePolicy::Policy::Preferred, QSizePolicy::Policy::Preferred);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(Keyboard->sizePolicy().hasHeightForWidth());
        Keyboard->setSizePolicy(sizePolicy);
        Keyboard->setWindowTitle(QString::fromUtf8("Keyboard"));
        Keyboard->setStyleSheet(QString::fromUtf8(""));
        verticalLayout = new QVBoxLayout(Keyboard);
        verticalLayout->setObjectName("verticalLayout");
        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName("horizontalLayout");
        lineEdit = new QLineEdit(Keyboard);
        lineEdit->setObjectName("lineEdit");
        lineEdit->setStyleSheet(QString::fromUtf8("color: rgb(0, 0, 0);\n"
"background-color: rgb(255, 255, 255);"));

        horizontalLayout->addWidget(lineEdit);

        backButton = new QPushButton(Keyboard);
        backButton->setObjectName("backButton");

        horizontalLayout->addWidget(backButton);

        clear = new QPushButton(Keyboard);
        clear->setObjectName("clear");

        horizontalLayout->addWidget(clear);


        verticalLayout->addLayout(horizontalLayout);

        gridLayout = new QGridLayout();
        gridLayout->setSpacing(2);
        gridLayout->setObjectName("gridLayout");
        Buttonq = new QPushButton(Keyboard);
        Buttonq->setObjectName("Buttonq");
        QSizePolicy sizePolicy1(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Expanding);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(Buttonq->sizePolicy().hasHeightForWidth());
        Buttonq->setSizePolicy(sizePolicy1);
        Buttonq->setMinimumSize(QSize(20, 0));

        gridLayout->addWidget(Buttonq, 1, 0, 1, 1);

        Buttonw = new QPushButton(Keyboard);
        Buttonw->setObjectName("Buttonw");
        sizePolicy1.setHeightForWidth(Buttonw->sizePolicy().hasHeightForWidth());
        Buttonw->setSizePolicy(sizePolicy1);
        Buttonw->setMinimumSize(QSize(20, 0));

        gridLayout->addWidget(Buttonw, 1, 1, 1, 1);

        Buttone = new QPushButton(Keyboard);
        Buttone->setObjectName("Buttone");
        sizePolicy1.setHeightForWidth(Buttone->sizePolicy().hasHeightForWidth());
        Buttone->setSizePolicy(sizePolicy1);
        Buttone->setMinimumSize(QSize(20, 0));

        gridLayout->addWidget(Buttone, 1, 2, 1, 1);

        Buttonr = new QPushButton(Keyboard);
        Buttonr->setObjectName("Buttonr");
        sizePolicy1.setHeightForWidth(Buttonr->sizePolicy().hasHeightForWidth());
        Buttonr->setSizePolicy(sizePolicy1);
        Buttonr->setMinimumSize(QSize(20, 0));

        gridLayout->addWidget(Buttonr, 1, 3, 1, 1);

        Buttont = new QPushButton(Keyboard);
        Buttont->setObjectName("Buttont");
        sizePolicy1.setHeightForWidth(Buttont->sizePolicy().hasHeightForWidth());
        Buttont->setSizePolicy(sizePolicy1);
        Buttont->setMinimumSize(QSize(20, 0));

        gridLayout->addWidget(Buttont, 1, 4, 1, 1);

        Buttony = new QPushButton(Keyboard);
        Buttony->setObjectName("Buttony");
        sizePolicy1.setHeightForWidth(Buttony->sizePolicy().hasHeightForWidth());
        Buttony->setSizePolicy(sizePolicy1);
        Buttony->setMinimumSize(QSize(20, 0));

        gridLayout->addWidget(Buttony, 1, 5, 1, 1);

        Buttonu = new QPushButton(Keyboard);
        Buttonu->setObjectName("Buttonu");
        sizePolicy1.setHeightForWidth(Buttonu->sizePolicy().hasHeightForWidth());
        Buttonu->setSizePolicy(sizePolicy1);
        Buttonu->setMinimumSize(QSize(20, 0));

        gridLayout->addWidget(Buttonu, 1, 6, 1, 1);

        Buttoni = new QPushButton(Keyboard);
        Buttoni->setObjectName("Buttoni");
        sizePolicy1.setHeightForWidth(Buttoni->sizePolicy().hasHeightForWidth());
        Buttoni->setSizePolicy(sizePolicy1);
        Buttoni->setMinimumSize(QSize(20, 0));

        gridLayout->addWidget(Buttoni, 1, 7, 1, 1);

        Buttono = new QPushButton(Keyboard);
        Buttono->setObjectName("Buttono");
        sizePolicy1.setHeightForWidth(Buttono->sizePolicy().hasHeightForWidth());
        Buttono->setSizePolicy(sizePolicy1);
        Buttono->setMinimumSize(QSize(20, 0));

        gridLayout->addWidget(Buttono, 1, 8, 1, 1);

        Buttons = new QPushButton(Keyboard);
        Buttons->setObjectName("Buttons");
        sizePolicy1.setHeightForWidth(Buttons->sizePolicy().hasHeightForWidth());
        Buttons->setSizePolicy(sizePolicy1);
        Buttons->setMinimumSize(QSize(20, 0));

        gridLayout->addWidget(Buttons, 3, 1, 1, 1);

        Buttond = new QPushButton(Keyboard);
        Buttond->setObjectName("Buttond");
        sizePolicy1.setHeightForWidth(Buttond->sizePolicy().hasHeightForWidth());
        Buttond->setSizePolicy(sizePolicy1);
        Buttond->setMinimumSize(QSize(20, 0));

        gridLayout->addWidget(Buttond, 3, 2, 1, 1);

        Buttonf = new QPushButton(Keyboard);
        Buttonf->setObjectName("Buttonf");
        sizePolicy1.setHeightForWidth(Buttonf->sizePolicy().hasHeightForWidth());
        Buttonf->setSizePolicy(sizePolicy1);
        Buttonf->setMinimumSize(QSize(20, 0));

        gridLayout->addWidget(Buttonf, 3, 3, 1, 1);

        Buttong = new QPushButton(Keyboard);
        Buttong->setObjectName("Buttong");
        sizePolicy1.setHeightForWidth(Buttong->sizePolicy().hasHeightForWidth());
        Buttong->setSizePolicy(sizePolicy1);
        Buttong->setMinimumSize(QSize(20, 0));

        gridLayout->addWidget(Buttong, 3, 4, 1, 1);

        Buttonh = new QPushButton(Keyboard);
        Buttonh->setObjectName("Buttonh");
        sizePolicy1.setHeightForWidth(Buttonh->sizePolicy().hasHeightForWidth());
        Buttonh->setSizePolicy(sizePolicy1);
        Buttonh->setMinimumSize(QSize(20, 0));

        gridLayout->addWidget(Buttonh, 3, 5, 1, 1);

        Buttonj = new QPushButton(Keyboard);
        Buttonj->setObjectName("Buttonj");
        sizePolicy1.setHeightForWidth(Buttonj->sizePolicy().hasHeightForWidth());
        Buttonj->setSizePolicy(sizePolicy1);
        Buttonj->setMinimumSize(QSize(20, 0));

        gridLayout->addWidget(Buttonj, 3, 6, 1, 1);

        Buttonl = new QPushButton(Keyboard);
        Buttonl->setObjectName("Buttonl");
        sizePolicy1.setHeightForWidth(Buttonl->sizePolicy().hasHeightForWidth());
        Buttonl->setSizePolicy(sizePolicy1);
        Buttonl->setMinimumSize(QSize(20, 0));

        gridLayout->addWidget(Buttonl, 3, 8, 1, 1);

        Buttonz = new QPushButton(Keyboard);
        Buttonz->setObjectName("Buttonz");
        sizePolicy1.setHeightForWidth(Buttonz->sizePolicy().hasHeightForWidth());
        Buttonz->setSizePolicy(sizePolicy1);
        Buttonz->setMinimumSize(QSize(20, 0));

        gridLayout->addWidget(Buttonz, 4, 1, 1, 1);

        Buttonx = new QPushButton(Keyboard);
        Buttonx->setObjectName("Buttonx");
        sizePolicy1.setHeightForWidth(Buttonx->sizePolicy().hasHeightForWidth());
        Buttonx->setSizePolicy(sizePolicy1);
        Buttonx->setMinimumSize(QSize(20, 0));

        gridLayout->addWidget(Buttonx, 4, 2, 1, 1);

        Buttonv = new QPushButton(Keyboard);
        Buttonv->setObjectName("Buttonv");
        sizePolicy1.setHeightForWidth(Buttonv->sizePolicy().hasHeightForWidth());
        Buttonv->setSizePolicy(sizePolicy1);
        Buttonv->setMinimumSize(QSize(20, 0));

        gridLayout->addWidget(Buttonv, 4, 4, 1, 1);

        Buttonb = new QPushButton(Keyboard);
        Buttonb->setObjectName("Buttonb");
        sizePolicy1.setHeightForWidth(Buttonb->sizePolicy().hasHeightForWidth());
        Buttonb->setSizePolicy(sizePolicy1);
        Buttonb->setMinimumSize(QSize(20, 0));

        gridLayout->addWidget(Buttonb, 4, 5, 1, 1);

        Buttonn = new QPushButton(Keyboard);
        Buttonn->setObjectName("Buttonn");
        sizePolicy1.setHeightForWidth(Buttonn->sizePolicy().hasHeightForWidth());
        Buttonn->setSizePolicy(sizePolicy1);
        Buttonn->setMinimumSize(QSize(20, 0));

        gridLayout->addWidget(Buttonn, 4, 6, 1, 1);

        Buttonm = new QPushButton(Keyboard);
        Buttonm->setObjectName("Buttonm");
        sizePolicy1.setHeightForWidth(Buttonm->sizePolicy().hasHeightForWidth());
        Buttonm->setSizePolicy(sizePolicy1);
        Buttonm->setMinimumSize(QSize(20, 0));

        gridLayout->addWidget(Buttonm, 4, 7, 1, 1);

        char_2 = new QPushButton(Keyboard);
        char_2->setObjectName("char_2");
        sizePolicy1.setHeightForWidth(char_2->sizePolicy().hasHeightForWidth());
        char_2->setSizePolicy(sizePolicy1);
        char_2->setMinimumSize(QSize(20, 0));
        char_2->setCheckable(true);
        char_2->setChecked(false);

        gridLayout->addWidget(char_2, 4, 8, 2, 1);

        space = new QPushButton(Keyboard);
        space->setObjectName("space");
        space->setMinimumSize(QSize(20, 50));

        gridLayout->addWidget(space, 5, 1, 1, 7);

        Button0 = new QPushButton(Keyboard);
        Button0->setObjectName("Button0");
        sizePolicy1.setHeightForWidth(Button0->sizePolicy().hasHeightForWidth());
        Button0->setSizePolicy(sizePolicy1);
        Button0->setMinimumSize(QSize(20, 0));

        gridLayout->addWidget(Button0, 0, 0, 1, 1);

        Button1 = new QPushButton(Keyboard);
        Button1->setObjectName("Button1");
        sizePolicy1.setHeightForWidth(Button1->sizePolicy().hasHeightForWidth());
        Button1->setSizePolicy(sizePolicy1);
        Button1->setMinimumSize(QSize(20, 0));

        gridLayout->addWidget(Button1, 0, 1, 1, 1);

        Button2 = new QPushButton(Keyboard);
        Button2->setObjectName("Button2");
        sizePolicy1.setHeightForWidth(Button2->sizePolicy().hasHeightForWidth());
        Button2->setSizePolicy(sizePolicy1);
        Button2->setMinimumSize(QSize(20, 0));

        gridLayout->addWidget(Button2, 0, 2, 1, 1);

        Button3 = new QPushButton(Keyboard);
        Button3->setObjectName("Button3");
        sizePolicy1.setHeightForWidth(Button3->sizePolicy().hasHeightForWidth());
        Button3->setSizePolicy(sizePolicy1);
        Button3->setMinimumSize(QSize(20, 0));

        gridLayout->addWidget(Button3, 0, 3, 1, 1);

        Button4 = new QPushButton(Keyboard);
        Button4->setObjectName("Button4");
        sizePolicy1.setHeightForWidth(Button4->sizePolicy().hasHeightForWidth());
        Button4->setSizePolicy(sizePolicy1);
        Button4->setMinimumSize(QSize(20, 0));

        gridLayout->addWidget(Button4, 0, 4, 1, 1);

        Button5 = new QPushButton(Keyboard);
        Button5->setObjectName("Button5");
        sizePolicy1.setHeightForWidth(Button5->sizePolicy().hasHeightForWidth());
        Button5->setSizePolicy(sizePolicy1);
        Button5->setMinimumSize(QSize(20, 0));

        gridLayout->addWidget(Button5, 0, 5, 1, 1);

        Button6 = new QPushButton(Keyboard);
        Button6->setObjectName("Button6");
        sizePolicy1.setHeightForWidth(Button6->sizePolicy().hasHeightForWidth());
        Button6->setSizePolicy(sizePolicy1);
        Button6->setMinimumSize(QSize(20, 0));

        gridLayout->addWidget(Button6, 0, 6, 1, 1);

        Button7 = new QPushButton(Keyboard);
        Button7->setObjectName("Button7");
        sizePolicy1.setHeightForWidth(Button7->sizePolicy().hasHeightForWidth());
        Button7->setSizePolicy(sizePolicy1);
        Button7->setMinimumSize(QSize(20, 0));

        gridLayout->addWidget(Button7, 0, 7, 1, 1);

        Button8 = new QPushButton(Keyboard);
        Button8->setObjectName("Button8");
        sizePolicy1.setHeightForWidth(Button8->sizePolicy().hasHeightForWidth());
        Button8->setSizePolicy(sizePolicy1);
        Button8->setMinimumSize(QSize(20, 0));

        gridLayout->addWidget(Button8, 0, 8, 1, 1);

        Button9 = new QPushButton(Keyboard);
        Button9->setObjectName("Button9");
        sizePolicy1.setHeightForWidth(Button9->sizePolicy().hasHeightForWidth());
        Button9->setSizePolicy(sizePolicy1);
        Button9->setMinimumSize(QSize(20, 0));

        gridLayout->addWidget(Button9, 0, 9, 1, 1);

        Buttonp = new QPushButton(Keyboard);
        Buttonp->setObjectName("Buttonp");
        sizePolicy1.setHeightForWidth(Buttonp->sizePolicy().hasHeightForWidth());
        Buttonp->setSizePolicy(sizePolicy1);
        Buttonp->setMinimumSize(QSize(20, 0));

        gridLayout->addWidget(Buttonp, 1, 9, 1, 1);

        enterButton = new QPushButton(Keyboard);
        enterButton->setObjectName("enterButton");
        sizePolicy1.setHeightForWidth(enterButton->sizePolicy().hasHeightForWidth());
        enterButton->setSizePolicy(sizePolicy1);
        enterButton->setMinimumSize(QSize(20, 0));

        gridLayout->addWidget(enterButton, 3, 9, 3, 1);

        Buttona = new QPushButton(Keyboard);
        Buttona->setObjectName("Buttona");
        sizePolicy1.setHeightForWidth(Buttona->sizePolicy().hasHeightForWidth());
        Buttona->setSizePolicy(sizePolicy1);
        Buttona->setMinimumSize(QSize(20, 0));

        gridLayout->addWidget(Buttona, 3, 0, 1, 1);

        shift = new QPushButton(Keyboard);
        shift->setObjectName("shift");
        sizePolicy1.setHeightForWidth(shift->sizePolicy().hasHeightForWidth());
        shift->setSizePolicy(sizePolicy1);
        shift->setMinimumSize(QSize(20, 0));
        shift->setStyleSheet(QString::fromUtf8(""));
        shift->setCheckable(false);

        gridLayout->addWidget(shift, 4, 0, 2, 1);

        Buttonc = new QPushButton(Keyboard);
        Buttonc->setObjectName("Buttonc");
        sizePolicy1.setHeightForWidth(Buttonc->sizePolicy().hasHeightForWidth());
        Buttonc->setSizePolicy(sizePolicy1);
        Buttonc->setMinimumSize(QSize(20, 0));

        gridLayout->addWidget(Buttonc, 4, 3, 1, 1);

        Buttonk = new QPushButton(Keyboard);
        Buttonk->setObjectName("Buttonk");
        sizePolicy1.setHeightForWidth(Buttonk->sizePolicy().hasHeightForWidth());
        Buttonk->setSizePolicy(sizePolicy1);
        Buttonk->setMinimumSize(QSize(20, 0));

        gridLayout->addWidget(Buttonk, 3, 7, 1, 1);


        verticalLayout->addLayout(gridLayout);


        retranslateUi(Keyboard);

        QMetaObject::connectSlotsByName(Keyboard);
    } // setupUi

    void retranslateUi(QWidget *Keyboard)
    {
        backButton->setText(QCoreApplication::translate("Keyboard", "Back", nullptr));
        clear->setText(QCoreApplication::translate("Keyboard", "Clear", nullptr));
        Buttonq->setText(QCoreApplication::translate("Keyboard", "q", nullptr));
        Buttonw->setText(QCoreApplication::translate("Keyboard", "w", nullptr));
        Buttone->setText(QCoreApplication::translate("Keyboard", "e", nullptr));
        Buttonr->setText(QCoreApplication::translate("Keyboard", "r", nullptr));
        Buttont->setText(QCoreApplication::translate("Keyboard", "t", nullptr));
        Buttony->setText(QCoreApplication::translate("Keyboard", "y", nullptr));
        Buttonu->setText(QCoreApplication::translate("Keyboard", "u", nullptr));
        Buttoni->setText(QCoreApplication::translate("Keyboard", "i", nullptr));
        Buttono->setText(QCoreApplication::translate("Keyboard", "o", nullptr));
        Buttons->setText(QCoreApplication::translate("Keyboard", "s", nullptr));
        Buttond->setText(QCoreApplication::translate("Keyboard", "d", nullptr));
        Buttonf->setText(QCoreApplication::translate("Keyboard", "f", nullptr));
        Buttong->setText(QCoreApplication::translate("Keyboard", "g", nullptr));
        Buttonh->setText(QCoreApplication::translate("Keyboard", "h", nullptr));
        Buttonj->setText(QCoreApplication::translate("Keyboard", "j", nullptr));
        Buttonl->setText(QCoreApplication::translate("Keyboard", "l", nullptr));
        Buttonz->setText(QCoreApplication::translate("Keyboard", "z", nullptr));
        Buttonx->setText(QCoreApplication::translate("Keyboard", "x", nullptr));
        Buttonv->setText(QCoreApplication::translate("Keyboard", "v", nullptr));
        Buttonb->setText(QCoreApplication::translate("Keyboard", "b", nullptr));
        Buttonn->setText(QCoreApplication::translate("Keyboard", "n", nullptr));
        Buttonm->setText(QCoreApplication::translate("Keyboard", "m", nullptr));
        char_2->setText(QCoreApplication::translate("Keyboard", "Char", nullptr));
        space->setText(QCoreApplication::translate("Keyboard", "Space", nullptr));
        Button0->setText(QCoreApplication::translate("Keyboard", "0", nullptr));
        Button1->setText(QCoreApplication::translate("Keyboard", "1", nullptr));
        Button2->setText(QCoreApplication::translate("Keyboard", "2", nullptr));
        Button3->setText(QCoreApplication::translate("Keyboard", "3", nullptr));
        Button4->setText(QCoreApplication::translate("Keyboard", "4", nullptr));
        Button5->setText(QCoreApplication::translate("Keyboard", "5", nullptr));
        Button6->setText(QCoreApplication::translate("Keyboard", "6", nullptr));
        Button7->setText(QCoreApplication::translate("Keyboard", "7", nullptr));
        Button8->setText(QCoreApplication::translate("Keyboard", "8", nullptr));
        Button9->setText(QCoreApplication::translate("Keyboard", "9", nullptr));
        Buttonp->setText(QCoreApplication::translate("Keyboard", "p", nullptr));
        enterButton->setText(QCoreApplication::translate("Keyboard", "Enter", nullptr));
        Buttona->setText(QCoreApplication::translate("Keyboard", "a", nullptr));
        shift->setText(QCoreApplication::translate("Keyboard", "Shift", nullptr));
        Buttonc->setText(QCoreApplication::translate("Keyboard", "c", nullptr));
        Buttonk->setText(QCoreApplication::translate("Keyboard", "k", nullptr));
        (void)Keyboard;
    } // retranslateUi

};

namespace Ui {
    class Keyboard: public Ui_Keyboard {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_KEYBOARD_H
