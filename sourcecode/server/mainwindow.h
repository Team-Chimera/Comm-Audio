#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <iostream>
#include "session.h"

using namespace std;

namespace Ui
{
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void setupConnections();

public slots:
    void openFile();
    bool initialConnect();

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
