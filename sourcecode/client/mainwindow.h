#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <iostream>
#include <vector>

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
    bool initialConnect();
    void clearListeners();
    void updateListeners(std::string);
    void updateNowPlaying(std::vector<std::string>);

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
