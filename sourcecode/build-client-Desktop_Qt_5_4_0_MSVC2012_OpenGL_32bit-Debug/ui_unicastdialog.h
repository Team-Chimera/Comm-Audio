/********************************************************************************
** Form generated from reading UI file 'unicastdialog.ui'
**
** Created by: Qt User Interface Compiler version 5.4.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_UNICASTDIALOG_H
#define UI_UNICASTDIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QDialog>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>

QT_BEGIN_NAMESPACE

class Ui_unicastDialog
{
public:
    QPushButton *playUnicast;
    QPushButton *DownloadSong;
    QLabel *song;

    void setupUi(QDialog *unicastDialog)
    {
        if (unicastDialog->objectName().isEmpty())
            unicastDialog->setObjectName(QStringLiteral("unicastDialog"));
        unicastDialog->resize(400, 74);
        playUnicast = new QPushButton(unicastDialog);
        playUnicast->setObjectName(QStringLiteral("playUnicast"));
        playUnicast->setGeometry(QRect(40, 30, 121, 31));
        DownloadSong = new QPushButton(unicastDialog);
        DownloadSong->setObjectName(QStringLiteral("DownloadSong"));
        DownloadSong->setGeometry(QRect(220, 30, 111, 31));
        song = new QLabel(unicastDialog);
        song->setObjectName(QStringLiteral("song"));
        song->setGeometry(QRect(160, 0, 91, 21));
        song->setStyleSheet(QStringLiteral("font-weight: bold;"));

        retranslateUi(unicastDialog);

        QMetaObject::connectSlotsByName(unicastDialog);
    } // setupUi

    void retranslateUi(QDialog *unicastDialog)
    {
        unicastDialog->setWindowTitle(QApplication::translate("unicastDialog", "Dialog", 0));
        playUnicast->setText(QApplication::translate("unicastDialog", "Play", 0));
        DownloadSong->setText(QApplication::translate("unicastDialog", "Download", 0));
        song->setText(QApplication::translate("unicastDialog", "TextLabel", 0));
    } // retranslateUi

};

namespace Ui {
    class unicastDialog: public Ui_unicastDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_UNICASTDIALOG_H
