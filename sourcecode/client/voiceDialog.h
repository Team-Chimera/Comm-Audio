#ifndef VOICEDIALOG_H
#define VOICEDIALOG_H
#define NOMINMAX

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
    bool startVoiceChat();

private:
    Ui::voiceDialog *ui;
    std::string clickedClient;
};

DWORD WINAPI voiceChat(LPVOID);

#endif // VOICEDIALOG_H
