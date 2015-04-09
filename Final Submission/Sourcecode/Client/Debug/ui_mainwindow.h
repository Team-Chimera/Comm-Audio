/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 5.4.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSlider>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralWidget;
    QSlider *volume;
    QTabWidget *tabWidget;
    QWidget *Multicast;
    QLabel *label_2;
    QLabel *label_3;
    QLabel *label_4;
    QLabel *artistName;
    QLabel *songName;
    QWidget *Library;
    QListWidget *songs;
    QLabel *label;
    QListWidget *listeners;
    QLabel *volumeIcon;
    QPushButton *multicastButton;
    QMenuBar *menuBar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QStringLiteral("MainWindow"));
        MainWindow->resize(775, 421);
        QPalette palette;
        QBrush brush(QColor(255, 255, 255, 255));
        brush.setStyle(Qt::SolidPattern);
        palette.setBrush(QPalette::Active, QPalette::WindowText, brush);
        QBrush brush1(QColor(36, 49, 58, 255));
        brush1.setStyle(Qt::SolidPattern);
        palette.setBrush(QPalette::Active, QPalette::Button, brush1);
        QBrush brush2(QColor(183, 240, 255, 255));
        brush2.setStyle(Qt::SolidPattern);
        palette.setBrush(QPalette::Active, QPalette::Light, brush2);
        QBrush brush3(QColor(149, 206, 221, 255));
        brush3.setStyle(Qt::SolidPattern);
        palette.setBrush(QPalette::Active, QPalette::Midlight, brush3);
        QBrush brush4(QColor(57, 86, 94, 255));
        brush4.setStyle(Qt::SolidPattern);
        palette.setBrush(QPalette::Active, QPalette::Dark, brush4);
        QBrush brush5(QColor(76, 115, 125, 255));
        brush5.setStyle(Qt::SolidPattern);
        palette.setBrush(QPalette::Active, QPalette::Mid, brush5);
        palette.setBrush(QPalette::Active, QPalette::Text, brush);
        palette.setBrush(QPalette::Active, QPalette::BrightText, brush);
        palette.setBrush(QPalette::Active, QPalette::ButtonText, brush);
        palette.setBrush(QPalette::Active, QPalette::Base, brush1);
        palette.setBrush(QPalette::Active, QPalette::Window, brush1);
        QBrush brush6(QColor(0, 0, 0, 255));
        brush6.setStyle(Qt::SolidPattern);
        palette.setBrush(QPalette::Active, QPalette::Shadow, brush6);
        QBrush brush7(QColor(185, 213, 221, 255));
        brush7.setStyle(Qt::SolidPattern);
        palette.setBrush(QPalette::Active, QPalette::AlternateBase, brush7);
        QBrush brush8(QColor(255, 255, 220, 255));
        brush8.setStyle(Qt::SolidPattern);
        palette.setBrush(QPalette::Active, QPalette::ToolTipBase, brush8);
        palette.setBrush(QPalette::Active, QPalette::ToolTipText, brush6);
        palette.setBrush(QPalette::Inactive, QPalette::WindowText, brush);
        palette.setBrush(QPalette::Inactive, QPalette::Button, brush1);
        palette.setBrush(QPalette::Inactive, QPalette::Light, brush2);
        palette.setBrush(QPalette::Inactive, QPalette::Midlight, brush3);
        palette.setBrush(QPalette::Inactive, QPalette::Dark, brush4);
        palette.setBrush(QPalette::Inactive, QPalette::Mid, brush5);
        palette.setBrush(QPalette::Inactive, QPalette::Text, brush);
        palette.setBrush(QPalette::Inactive, QPalette::BrightText, brush);
        palette.setBrush(QPalette::Inactive, QPalette::ButtonText, brush);
        palette.setBrush(QPalette::Inactive, QPalette::Base, brush1);
        palette.setBrush(QPalette::Inactive, QPalette::Window, brush1);
        palette.setBrush(QPalette::Inactive, QPalette::Shadow, brush6);
        palette.setBrush(QPalette::Inactive, QPalette::AlternateBase, brush7);
        palette.setBrush(QPalette::Inactive, QPalette::ToolTipBase, brush8);
        palette.setBrush(QPalette::Inactive, QPalette::ToolTipText, brush6);
        palette.setBrush(QPalette::Disabled, QPalette::WindowText, brush);
        palette.setBrush(QPalette::Disabled, QPalette::Button, brush1);
        palette.setBrush(QPalette::Disabled, QPalette::Light, brush2);
        palette.setBrush(QPalette::Disabled, QPalette::Midlight, brush3);
        palette.setBrush(QPalette::Disabled, QPalette::Dark, brush4);
        palette.setBrush(QPalette::Disabled, QPalette::Mid, brush5);
        palette.setBrush(QPalette::Disabled, QPalette::Text, brush);
        palette.setBrush(QPalette::Disabled, QPalette::BrightText, brush);
        palette.setBrush(QPalette::Disabled, QPalette::ButtonText, brush);
        palette.setBrush(QPalette::Disabled, QPalette::Base, brush1);
        palette.setBrush(QPalette::Disabled, QPalette::Window, brush1);
        palette.setBrush(QPalette::Disabled, QPalette::Shadow, brush6);
        QBrush brush9(QColor(115, 172, 188, 255));
        brush9.setStyle(Qt::SolidPattern);
        palette.setBrush(QPalette::Disabled, QPalette::AlternateBase, brush9);
        palette.setBrush(QPalette::Disabled, QPalette::ToolTipBase, brush8);
        palette.setBrush(QPalette::Disabled, QPalette::ToolTipText, brush6);
        MainWindow->setPalette(palette);
        MainWindow->setAcceptDrops(false);
        MainWindow->setAutoFillBackground(false);
        MainWindow->setStyleSheet(QLatin1String("* {\n"
"	background: #24313a;\n"
"	color: #FFF;\n"
"	border: 1px solid #5A5A5A;\n"
"}\n"
"\n"
"QWidget::item:selected {\n"
"	background: #1fbc34;\n"
"}\n"
"\n"
"QCheckBox, QRadioButton {\n"
"	border: none;\n"
"}\n"
"\n"
"QRadioButton::indicator, QCheckBox::indicator {\n"
"	width: 13px;\n"
"	height: 13px;\n"
"}\n"
"\n"
"QRadioButton::indicator::unchecked, QCheckBox::indicator::unchecked {\n"
"	border: 1px solid #5A5A5A;\n"
"	background: none;\n"
"}\n"
"\n"
"QRadioButton::indicator:unchecked:hover, QCheckBox::indicator:unchecked:hover {\n"
"	border: 1px solid #DDDDDD;\n"
"}\n"
"\n"
"QRadioButton::indicator::checked, QCheckBox::indicator::checked {\n"
"	border: 1px solid #5A5A5A;\n"
"	background: #5A5A5A;\n"
"}\n"
"\n"
"QRadioButton::indicator:checked:hover, QCheckBox::indicator:checked:hover {\n"
"	border: 1px solid #DDDDDD;\n"
"	background: #DDDDDD;\n"
"}\n"
"\n"
"QGroupBox {\n"
"	margin-top: 6px;\n"
"}\n"
"\n"
"QGroupBox::title {\n"
"	top: -7px;\n"
"	left: 7px;\n"
"}\n"
"  QScrollBar:vertical {\n"
"      borde"
                        "r: 2px solid grey;\n"
"      background: #32CC99;\n"
"      width: 15px;\n"
"      margin: 22px 0 22px 0;\n"
"  }\n"
"  QScrollBar::handle:vertical {\n"
"      background: white;\n"
"      min-height: 20px;\n"
"  }\n"
"  QScrollBar::add-line:vertical {\n"
"      border: 2px solid grey;\n"
"      background: #32CC99;\n"
"      height: 20px;\n"
"      subcontrol-position: bottom;\n"
"      subcontrol-origin: margin;\n"
"  }\n"
"\n"
"  QScrollBar::sub-line:vertical {\n"
"      border: 2px solid grey;\n"
"      background: #32CC99;\n"
"      height: 20px;\n"
"      subcontrol-position: top;\n"
"      subcontrol-origin: margin;\n"
"  }\n"
"  QScrollBar::up-arrow:vertical, QScrollBar::down-arrow:vertical {\n"
"      border: 2px solid grey;\n"
"      width: 3px;\n"
"      height: 3px;\n"
"      background: white;\n"
"  }\n"
"\n"
"  QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical {\n"
"      background: none;\n"
"  }\n"
"\n"
"QAbstractItemView {\n"
"	show-decoration-selected: 1;\n"
"	selection-background-"
                        "color: #3D7848;\n"
"	selection-color: #DDDDDD;\n"
"	alternate-background-color: #353535;\n"
"}\n"
"\n"
"QHeaderView {\n"
"	border: 1px solid #5A5A5A;\n"
"}\n"
"\n"
"QHeaderView::section {\n"
"	background: #191919;\n"
"	border: 1px solid #5A5A5A;\n"
"	padding: 4px;\n"
"}\n"
"\n"
"QHeaderView::section:selected, QHeaderView::section::checked {\n"
"	background: #353535;\n"
"}\n"
"\n"
"QTableView {\n"
"	gridline-color: #5A5A5A;\n"
"}\n"
"\n"
"QTabBar {\n"
"	margin-left: 2px;\n"
"}\n"
"\n"
"QTabBar::tab {\n"
"	border-radius: 0px;\n"
"	padding: 4px;\n"
"	margin: 4px;\n"
"}\n"
"\n"
"QTabBar::tab:selected {\n"
"	background: #5fd56f;\n"
"}\n"
"\n"
"QComboBox::down-arrow {\n"
"	border: 1px solid #5A5A5A;\n"
"	background: #353535;\n"
"}\n"
"\n"
"QComboBox::drop-down {\n"
"	border: 1px solid #5A5A5A;\n"
"	background: #353535;\n"
"}\n"
"\n"
"QComboBox::down-arrow {\n"
"	width: 3px;\n"
"	height: 3px;\n"
"	border: 1px solid #5A5A5A;\n"
"}\n"
"\n"
"QAbstractSpinBox {\n"
"	padding-right: 15px;\n"
"}\n"
"\n"
"QAbstractSpinBox::u"
                        "p-button, QAbstractSpinBox::down-button {\n"
"	border: 1px solid #5A5A5A;\n"
"	background: #353535;\n"
"	subcontrol-origin: border;\n"
"}\n"
"\n"
"QAbstractSpinBox::up-arrow, QAbstractSpinBox::down-arrow {\n"
"	width: 3px;\n"
"	height: 3px;\n"
"	border: 1px solid #5A5A5A;\n"
"}\n"
"\n"
"QSlider {\n"
"	border: none;\n"
"}\n"
"\n"
"QSlider::groove:horizontal {\n"
"	height: 5px;\n"
"	margin: 4px 0px 4px 0px;\n"
"}\n"
"\n"
"QSlider::groove:vertical {\n"
"	width: 5px;\n"
"	margin: 0px 4px 0px 4px;\n"
"}\n"
"\n"
"QSlider::handle {\n"
"	border: 1px solid #5A5A5A;\n"
"	background: #353535;\n"
"}\n"
"\n"
"QSlider::handle:horizontal {\n"
"	width: 15px;\n"
"	margin: -4px 0px -4px 0px;\n"
"}\n"
"\n"
"QSlider::handle:vertical {\n"
"	height: 15px;\n"
"	margin: 0px -4px 0px -4px;\n"
"}\n"
"\n"
"QSlider::add-page:vertical, QSlider::sub-page:horizontal {\n"
"	background: #5fd56f;\n"
"}\n"
"\n"
"QSlider::sub-page:vertical, QSlider::add-page:horizontal {\n"
"	background: #a2a2a2;\n"
"}\n"
"\n"
"QLabel {\n"
"	border: none;\n"
"}\n"
""
                        "\n"
"QProgressBar {\n"
"	text-align: center;\n"
"}\n"
"\n"
"QProgressBar::chunk {\n"
"	width: 1px;\n"
"	background-color: #5fd56f;\n"
"}\n"
"\n"
"QMenu::separator {\n"
"	background: #CC33FF;\n"
"}\n"
"\n"
"QAbstractButton:hover {\n"
"	background: #74f385;\n"
"}\n"
"\n"
"QAbstractButton:pressed {\n"
"	background: #1fbc34;\n"
"}\n"
"\n"
"QAbstractButton {\n"
"	background: #5fd56f;\n"
"}"));
        centralWidget = new QWidget(MainWindow);
        centralWidget->setObjectName(QStringLiteral("centralWidget"));
        volume = new QSlider(centralWidget);
        volume->setObjectName(QStringLiteral("volume"));
        volume->setGeometry(QRect(380, 330, 160, 19));
        volume->setStyleSheet(QStringLiteral(""));
        volume->setMinimum(0);
        volume->setMaximum(100);
        volume->setSingleStep(5);
        volume->setPageStep(5);
        volume->setValue(50);
        volume->setSliderPosition(50);
        volume->setOrientation(Qt::Horizontal);
        volume->setInvertedAppearance(false);
        volume->setInvertedControls(false);
        volume->setTickPosition(QSlider::NoTicks);
        volume->setTickInterval(5);
        tabWidget = new QTabWidget(centralWidget);
        tabWidget->setObjectName(QStringLiteral("tabWidget"));
        tabWidget->setGeometry(QRect(20, 10, 531, 281));
        tabWidget->setAcceptDrops(false);
        tabWidget->setAutoFillBackground(false);
        tabWidget->setTabShape(QTabWidget::Rounded);
        tabWidget->setTabBarAutoHide(false);
        Multicast = new QWidget();
        Multicast->setObjectName(QStringLiteral("Multicast"));
        QFont font;
        font.setFamily(QStringLiteral("Segoe UI"));
        font.setBold(true);
        font.setWeight(75);
        Multicast->setFont(font);
        Multicast->setAutoFillBackground(false);
        label_2 = new QLabel(Multicast);
        label_2->setObjectName(QStringLiteral("label_2"));
        label_2->setGeometry(QRect(10, 20, 141, 16));
        label_2->setStyleSheet(QLatin1String("color: #5fd56f;\n"
"font-weight: bold;\n"
""));
        label_3 = new QLabel(Multicast);
        label_3->setObjectName(QStringLiteral("label_3"));
        label_3->setGeometry(QRect(80, 70, 55, 16));
        label_3->setStyleSheet(QLatin1String("color: #5fd56f; font-weight: bold;\n"
""));
        label_4 = new QLabel(Multicast);
        label_4->setObjectName(QStringLiteral("label_4"));
        label_4->setGeometry(QRect(80, 160, 55, 16));
        label_4->setStyleSheet(QLatin1String("color: #5fd56f;\n"
"font-weight: bold;\n"
""));
        artistName = new QLabel(Multicast);
        artistName->setObjectName(QStringLiteral("artistName"));
        artistName->setGeometry(QRect(140, 160, 371, 16));
        songName = new QLabel(Multicast);
        songName->setObjectName(QStringLiteral("songName"));
        songName->setGeometry(QRect(130, 70, 381, 16));
        tabWidget->addTab(Multicast, QString());
        Library = new QWidget();
        Library->setObjectName(QStringLiteral("Library"));
        songs = new QListWidget(Library);
        songs->setObjectName(QStringLiteral("songs"));
        songs->setGeometry(QRect(0, 0, 521, 241));
        tabWidget->addTab(Library, QString());
        label = new QLabel(centralWidget);
        label->setObjectName(QStringLiteral("label"));
        label->setGeometry(QRect(590, 10, 131, 21));
        label->setStyleSheet(QLatin1String("color: #5fd56f;\n"
"font-weight: bold;\n"
""));
        listeners = new QListWidget(centralWidget);
        listeners->setObjectName(QStringLiteral("listeners"));
        listeners->setGeometry(QRect(580, 30, 171, 281));
        volumeIcon = new QLabel(centralWidget);
        volumeIcon->setObjectName(QStringLiteral("volumeIcon"));
        volumeIcon->setGeometry(QRect(340, 330, 21, 21));
        volumeIcon->setPixmap(QPixmap(QString::fromUtf8("images/volume.png")));
        volumeIcon->setScaledContents(true);
        multicastButton = new QPushButton(centralWidget);
        multicastButton->setObjectName(QStringLiteral("multicastButton"));
        multicastButton->setGeometry(QRect(190, 300, 101, 91));
        multicastButton->setStyleSheet(QLatin1String("background: none;\n"
"border: none;"));
        MainWindow->setCentralWidget(centralWidget);
        menuBar = new QMenuBar(MainWindow);
        menuBar->setObjectName(QStringLiteral("menuBar"));
        menuBar->setGeometry(QRect(0, 0, 775, 28));
        MainWindow->setMenuBar(menuBar);

        retranslateUi(MainWindow);

        tabWidget->setCurrentIndex(1);


        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QApplication::translate("MainWindow", "MainWindow", 0));
        label_2->setText(QApplication::translate("MainWindow", "Now Playing:", 0));
        label_3->setText(QApplication::translate("MainWindow", "Song:", 0));
        label_4->setText(QApplication::translate("MainWindow", "Artist:", 0));
        artistName->setText(QString());
        songName->setText(QString());
        tabWidget->setTabText(tabWidget->indexOf(Multicast), QApplication::translate("MainWindow", "Now Playing", 0));
        tabWidget->setTabText(tabWidget->indexOf(Library), QApplication::translate("MainWindow", "Library", 0));
        label->setText(QApplication::translate("MainWindow", "Connected Users:", 0));
        volumeIcon->setText(QString());
        multicastButton->setText(QString());
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
