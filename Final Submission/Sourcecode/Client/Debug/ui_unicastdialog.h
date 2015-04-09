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
        unicastDialog->resize(403, 87);
        playUnicast = new QPushButton(unicastDialog);
        playUnicast->setObjectName(QStringLiteral("playUnicast"));
        playUnicast->setGeometry(QRect(90, 30, 51, 51));
        playUnicast->setStyleSheet(QLatin1String("background: none;\n"
"border: none;"));
        DownloadSong = new QPushButton(unicastDialog);
        DownloadSong->setObjectName(QStringLiteral("DownloadSong"));
        DownloadSong->setGeometry(QRect(260, 30, 51, 51));
        DownloadSong->setStyleSheet(QLatin1String("background: none;\n"
"border: none;"));
        song = new QLabel(unicastDialog);
        song->setObjectName(QStringLiteral("song"));
        song->setGeometry(QRect(10, 0, 391, 21));
        song->setStyleSheet(QLatin1String("color: #5fd56f;\n"
"font-weight: bold;\n"
""));
        song->setAlignment(Qt::AlignCenter);

        retranslateUi(unicastDialog);

        QMetaObject::connectSlotsByName(unicastDialog);
    } // setupUi

    void retranslateUi(QDialog *unicastDialog)
    {
        unicastDialog->setWindowTitle(QApplication::translate("unicastDialog", "Dialog", 0));
        playUnicast->setText(QString());
        DownloadSong->setText(QString());
        song->setText(QApplication::translate("unicastDialog", "TextLabel", 0));
    } // retranslateUi

};

namespace Ui {
    class unicastDialog: public Ui_unicastDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_UNICASTDIALOG_H
