/********************************************************************************
** Form generated from reading UI file 'voiceDialog.ui'
**
** Created by: Qt User Interface Compiler version 5.4.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_VOICEDIALOG_H
#define UI_VOICEDIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QDialog>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>

QT_BEGIN_NAMESPACE

class Ui_voiceDialog
{
public:
    QPushButton *connectVoice;
    QLabel *client;

    void setupUi(QDialog *voiceDialog)
    {
        if (voiceDialog->objectName().isEmpty())
            voiceDialog->setObjectName(QStringLiteral("voiceDialog"));
        voiceDialog->resize(400, 74);
        connectVoice = new QPushButton(voiceDialog);
        connectVoice->setObjectName(QStringLiteral("connectVoice"));
        connectVoice->setGeometry(QRect(180, 20, 51, 41));
        connectVoice->setStyleSheet(QLatin1String("background: none;\n"
"border: none;"));
        connectVoice->setFlat(true);
        client = new QLabel(voiceDialog);
        client->setObjectName(QStringLiteral("client"));
        client->setGeometry(QRect(10, 0, 391, 21));
        client->setStyleSheet(QLatin1String("color: #5fd56f;\n"
"font-weight: bold;\n"
""));
        client->setAlignment(Qt::AlignCenter);

        retranslateUi(voiceDialog);

        QMetaObject::connectSlotsByName(voiceDialog);
    } // setupUi

    void retranslateUi(QDialog *voiceDialog)
    {
        voiceDialog->setWindowTitle(QApplication::translate("voiceDialog", "Voice Chat", 0));
        connectVoice->setText(QString());
        client->setText(QApplication::translate("voiceDialog", "TextLabel", 0));
    } // retranslateUi

};

namespace Ui {
    class voiceDialog: public Ui_voiceDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_VOICEDIALOG_H
