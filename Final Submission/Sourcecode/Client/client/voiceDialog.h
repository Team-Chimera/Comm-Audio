#ifndef VOICEDIALOG_H
#define VOICEDIALOG_H

#include <QDialog>
#include <Windows.h>
#include "microphone.h"

namespace Ui {
class voiceDialog;
}

class voiceDialog : public QDialog
{
    Q_OBJECT

public:
    explicit voiceDialog(Microphone *, QWidget *parent = 0);
    ~voiceDialog();
    void setClientName(std::string);

public slots:
    bool triggerVoiceChat();
    void reject();

private:
    Ui::voiceDialog *ui;
    std::string clickedClient;
    bool connected;
};
#endif // VOICEDIALOG_H
