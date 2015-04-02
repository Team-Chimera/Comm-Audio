#include <QIODevice>
#include <QAudioFormat>
#include <QAudioDeviceInfo>
#include <QTimer>
#include <QDebug>
#include <QObject>

#include "microphone.h"

Microphone::Microphone() : QObject()
{
    QAudioFormat format;
    format.setSampleRate(8000); 
    format.setChannelCount(1);
    format.setSampleSize(8);
    format.setCodec("audio/pcm");
    format.setByteOrder(QAudioFormat::LittleEndian);
    format.setSampleType(QAudioFormat::UnSignedInt);
    
    QAudioDeviceInfo info = QAudioDeviceInfo::defaultInputDevice();
    if(!info.isFormatSupported(format))
    {
        qWarning() << "Default format not supported, finding next best...";
        format = info.nearestFormat(format);
    }
    socket = new QUdpSocket();
    socket->connectToHost("192.168.1.70", 8800);
    
    audio = new QAudioInput(format, this);
    connect(audio, SIGNAL(stateChanged(QAudio::State)), this, SLOT(handleStateChanged(QAudio::State)));
    
    audio->start(socket);
}

Microphone::~Microphone()
{
    audio->stop();
    socket->close();
    delete audio;
    delete socket;
}

void Microphone::handleStateChanged(QAudio::State newState)
{
    switch(newState)
    {
        case QAudio::StoppedState:
            if(audio->error() != QAudio::NoError)
            {
                // Error handling
                qWarning() << "Error found: Microphone";
                
            }
            else
            {
                // Finished recording
                qDebug() << "Finished: Microphone";
                
            }
        break;
        case QAudio::ActiveState:
            // Started recording - read from IO device
            qDebug() << "Active: Microphone";
        break;
        default:
            // ... other cases as appropriate
        break;
    }
}
