#include <iostream>
#include <string>
#include "unicastdialog.h"
#include "ui_unicastdialog.h"
#include "controlChannel.h"

using std::string;

unicastDialog::unicastDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::unicastDialog)
{
    ui->setupUi(this);

    //add the connection to the two buttons
    connect(ui->playUnicast, SIGNAL(pressed()), this, SLOT(requestUnicast()));
    connect(ui->DownloadSong, SIGNAL(pressed()), this, SLOT(requestSong()));
}

unicastDialog::~unicastDialog()
{
    delete ui;
}



void unicastDialog::requestDownload()
{

}


void unicastDialog::requestSong()
{

}

void unicastDialog::setSongName(std::string song)
{
    clickedSong = song;
    ui->song->setText(QString::fromStdString(song));
}
