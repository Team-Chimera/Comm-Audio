#ifndef UNICASTDIALOG_H
#define UNICASTDIALOG_H

#include <QDialog>

namespace Ui {
class unicastDialog;
}

class unicastDialog : public QDialog
{
    Q_OBJECT

public:
    explicit unicastDialog(QWidget *parent = 0);
    ~unicastDialog();
    void setSongName(std::string);

public slots:
    void unicastSong();
    void downloadSong();

private:
    Ui::unicastDialog *ui;
    std::string clickedSong;
};

#endif // UNICASTDIALOG_H
