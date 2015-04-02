#ifndef MICROPHONE
#define MICROPHONE

#include <QUdpSocket>
#include <QAudioInput>
#include <QAudio>

class Microphone : QObject
{
    Q_OBJECT
    
    public:
        Microphone();
        ~Microphone();
    
    public slots:
        void handleStateChanged(QAudio::State newState);
    
    private:
        QUdpSocket *socket;
        QAudioInput *audio;
};

#endif

