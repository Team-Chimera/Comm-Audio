#ifndef MICROPHONE
#define MICROPHONE

#define VOICE_CHAT_CHANNEL 8800

#include <QUdpSocket>
#include <QAudioInput>
#include <QAudio>
#include <QString>

/*------------------------------------------------------------------------------
-- CLASS: Microphone
--
-- DATE: April 1, 2015
--
-- DESIGNER: Julian Brandrick
--
-- PROGRAMMER: Julian Brandrick
--
-- INTERFACE: class Microphone extends QObject
--
-- NOTES:
--  Records audio from the default input device and sends it to through a UDP 
--   socket.
------------------------------------------------------------------------------*/
class Microphone : QObject
{
    Q_OBJECT
    
    public:
        Microphone();
        ~Microphone();
        void startVoice(QString);
        void stopVoice();
    
    public slots:
        void handleStateChanged(QAudio::State newState);
    
    private:
        QUdpSocket *socket;
        QAudioInput *audio;
};

#endif

