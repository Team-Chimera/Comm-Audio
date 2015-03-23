/********************************************************************************
** Form generated from reading UI file 'dialog.ui'
**
** Created by: Qt User Interface Compiler version 5.4.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_DIALOG_H
#define UI_DIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QDialog>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>

QT_BEGIN_NAMESPACE

class Ui_Dialog
{
public:
    QLineEdit *addressField;
    QLabel *address;
    QLabel *title;
    QPushButton *connectButton;

    void setupUi(QDialog *Dialog)
    {
        if (Dialog->objectName().isEmpty())
            Dialog->setObjectName(QStringLiteral("Dialog"));
        Dialog->resize(400, 300);
        Dialog->setAutoFillBackground(false);
        Dialog->setStyleSheet(QStringLiteral(""));
        addressField = new QLineEdit(Dialog);
        addressField->setObjectName(QStringLiteral("addressField"));
        addressField->setGeometry(QRect(70, 100, 281, 21));
        addressField->setStyleSheet(QStringLiteral(""));
        addressField->setClearButtonEnabled(true);
        address = new QLabel(Dialog);
        address->setObjectName(QStringLiteral("address"));
        address->setGeometry(QRect(10, 100, 55, 16));
        title = new QLabel(Dialog);
        title->setObjectName(QStringLiteral("title"));
        title->setGeometry(QRect(10, 20, 381, 20));
        QFont font;
        font.setFamily(QStringLiteral("Proxima Nova Rg"));
        font.setPointSize(14);
        font.setBold(true);
        font.setWeight(75);
        title->setFont(font);
        title->setStyleSheet(QStringLiteral(""));
        title->setTextFormat(Qt::RichText);
        title->setAlignment(Qt::AlignCenter);
        title->setWordWrap(false);
        connectButton = new QPushButton(Dialog);
        connectButton->setObjectName(QStringLiteral("connectButton"));
        connectButton->setGeometry(QRect(90, 160, 211, 41));
        connectButton->setAutoFillBackground(false);
        connectButton->setStyleSheet(QStringLiteral(""));
        connectButton->setDefault(false);
        connectButton->setFlat(false);
        QWidget::setTabOrder(addressField, connectButton);

        retranslateUi(Dialog);

        QMetaObject::connectSlotsByName(Dialog);
    } // setupUi

    void retranslateUi(QDialog *Dialog)
    {
        Dialog->setWindowTitle(QApplication::translate("Dialog", "Chat++ Connector", 0));
        addressField->setPlaceholderText(QString());
        address->setText(QApplication::translate("Dialog", "Server", 0));
        title->setText(QApplication::translate("Dialog", "Audio Streamer", 0));
        connectButton->setText(QApplication::translate("Dialog", "Connect", 0));
    } // retranslateUi

};

namespace Ui {
    class Dialog: public Ui_Dialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_DIALOG_H
