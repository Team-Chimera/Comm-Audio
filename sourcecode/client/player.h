#ifndef PLAYER
#define PLAYER

#include <qudpsocket.h>
#include <QAudioOutput>
#include <QAudio>
#include <QObject>

#define VOICE_CHAT_CHANNEL 8800

/*------------------------------------------------------------------------------
-- CLASS: Player
--
-- DATE: April 1, 2015
--
-- DESIGNER: Julian Brandrick
--
-- PROGRAMMER: Julian Brandrick
--
-- INTERFACE: class Player extends QObject
--
-- NOTES:
--  Receives recorded audio from a UDP socket and plays it using the default
--   audio output device.
------------------------------------------------------------------------------*/
class Player : QObject
{
    Q_OBJECT
    
    public:
        Player();
        ~Player();
        
    public slots:
        void handleStateChanged(QAudio::State newState);
        void playData();
        void startVoicePlay();
        void stopVoicePlay();
        
    private:
        QUdpSocket *socket;
        QAudioOutput *audio;
        QIODevice *device;
};

#endif

