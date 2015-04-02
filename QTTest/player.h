#ifndef PLAYER
#define PLAYER

#include <QUdpSocket>
#include <QAudioOutput>
#include <QAudio>
#include <QObject>

class Player : QObject
{
    Q_OBJECT
    
    public:
        Player();
        ~Player();
        
    public slots:
        void handleStateChanged(QAudio::State newState);
        void playData();
        
    private:
        QUdpSocket *socket;
        QAudioOutput *audio;
        QIODevice *device;
};

#endif

