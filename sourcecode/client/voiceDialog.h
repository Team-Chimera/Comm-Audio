#ifndef VOICEDIALOG_H
#define VOICEDIALOG_H

#include <QDialog>

namespace Ui {
class voiceDialog;
}

class voiceDialog : public QDialog
{
    Q_OBJECT

public:
    explicit voiceDialog(QWidget *parent = 0);
    ~voiceDialog();
    void setClientName(std::string);

public slots:
    bool startVoiceChat();

private:
    Ui::voiceDialog *ui;
    std::string clickedClient;
};

#endif // VOICEDIALOG_H
