/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 6.8.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QStackedWidget>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralwidget;
    QStackedWidget *stackedWidget;
    QWidget *page;
    QGridLayout *gridLayout;
    QLabel *label;
    QVBoxLayout *verticalLayout;
    QPushButton *btn_Arcade;
    QPushButton *btn_Learn;
    QPushButton *btn_History;
    QPushButton *btn_Exit;
    QWidget *page_2;
    QLabel *label_;
    QLabel *label_Limit;
    QLabel *label_Speed;
    QPushButton *btn_Back;
    QMenuBar *menubar;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName("MainWindow");
        MainWindow->resize(800, 600);
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName("centralwidget");
        stackedWidget = new QStackedWidget(centralwidget);
        stackedWidget->setObjectName("stackedWidget");
        stackedWidget->setGeometry(QRect(100, 40, 461, 291));
        page = new QWidget();
        page->setObjectName("page");
        gridLayout = new QGridLayout(page);
        gridLayout->setObjectName("gridLayout");
        label = new QLabel(page);
        label->setObjectName("label");
        label->setAlignment(Qt::AlignmentFlag::AlignCenter);

        gridLayout->addWidget(label, 0, 0, 1, 1);

        verticalLayout = new QVBoxLayout();
        verticalLayout->setObjectName("verticalLayout");
        btn_Arcade = new QPushButton(page);
        btn_Arcade->setObjectName("btn_Arcade");

        verticalLayout->addWidget(btn_Arcade);

        btn_Learn = new QPushButton(page);
        btn_Learn->setObjectName("btn_Learn");

        verticalLayout->addWidget(btn_Learn);

        btn_History = new QPushButton(page);
        btn_History->setObjectName("btn_History");

        verticalLayout->addWidget(btn_History);

        btn_Exit = new QPushButton(page);
        btn_Exit->setObjectName("btn_Exit");

        verticalLayout->addWidget(btn_Exit);


        gridLayout->addLayout(verticalLayout, 1, 0, 1, 1);

        stackedWidget->addWidget(page);
        page_2 = new QWidget();
        page_2->setObjectName("page_2");
        label_ = new QLabel(page_2);
        label_->setObjectName("label_");
        label_->setGeometry(QRect(308, 6, 131, 16));
        label_Limit = new QLabel(page_2);
        label_Limit->setObjectName("label_Limit");
        label_Limit->setGeometry(QRect(157, 6, 141, 16));
        label_Speed = new QLabel(page_2);
        label_Speed->setObjectName("label_Speed");
        label_Speed->setGeometry(QRect(6, 6, 141, 16));
        btn_Back = new QPushButton(page_2);
        btn_Back->setObjectName("btn_Back");
        btn_Back->setGeometry(QRect(350, 230, 101, 51));
        stackedWidget->addWidget(page_2);
        MainWindow->setCentralWidget(centralwidget);
        menubar = new QMenuBar(MainWindow);
        menubar->setObjectName("menubar");
        menubar->setGeometry(QRect(0, 0, 800, 18));
        MainWindow->setMenuBar(menubar);
        statusbar = new QStatusBar(MainWindow);
        statusbar->setObjectName("statusbar");
        MainWindow->setStatusBar(statusbar);

        retranslateUi(MainWindow);

        stackedWidget->setCurrentIndex(0);


        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "MainWindow", nullptr));
        label->setText(QCoreApplication::translate("MainWindow", "Phantom Drive", nullptr));
        btn_Arcade->setText(QCoreApplication::translate("MainWindow", "\347\253\236\346\212\200\346\250\241\345\274\217", nullptr));
        btn_Learn->setText(QCoreApplication::translate("MainWindow", "\345\255\246\344\271\240\346\250\241\345\274\217", nullptr));
        btn_History->setText(QCoreApplication::translate("MainWindow", "\345\216\206\345\217\262\350\256\260\345\275\225\347\234\213\346\235\277", nullptr));
        btn_Exit->setText(QCoreApplication::translate("MainWindow", "\351\200\200\345\207\272\347\225\214\351\235\242", nullptr));
        label_->setText(QCoreApplication::translate("MainWindow", "\347\272\242\347\273\277\347\201\257\357\274\232", nullptr));
        label_Limit->setText(QCoreApplication::translate("MainWindow", "\351\231\220\351\200\237\357\274\232", nullptr));
        label_Speed->setText(QCoreApplication::translate("MainWindow", "\350\275\246\351\200\237\357\274\232", nullptr));
        btn_Back->setText(QCoreApplication::translate("MainWindow", "\350\277\224\345\233\236\344\270\273\350\217\234\345\215\225", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
