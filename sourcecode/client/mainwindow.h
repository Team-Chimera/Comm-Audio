#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <iostream>
#include <vector>
#include <QListWidgetItem>

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
    void addClient(std::string);
    void addSongToLibrary(std:: string);


public slots:
   // bool initialConnect();
    void clearListeners();
    void updateListeners(std::string);
    void updateNowPlaying(std::vector<std::string>);
    void openSongMenu(QListWidgetItem *);
    void openVoiceMenu(QListWidgetItem *);

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
