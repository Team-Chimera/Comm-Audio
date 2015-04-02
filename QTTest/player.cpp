#include <QAudioFormat>
#include <QAudioDeviceInfo>
#include <QIODevice>
#include <QByteArray>
#include <QDebug>

#include "player.h"

Player::Player() : QObject()
{
    QAudioFormat format;
    format.setSampleRate(8000);
    format.setChannelCount(1);
    format.setSampleSize(8);
    format.setCodec("audio/pcm");
    format.setByteOrder(QAudioFormat::LittleEndian);
    format.setSampleType(QAudioFormat::UnSignedInt);
    QAudioDeviceInfo info(QAudioDeviceInfo::defaultOutputDevice());
    if (!info.isFormatSupported(format)) 
    {
        qWarning() << "Raw audio format not supported by backend, cannot play audio.";
        format = info.nearestFormat(format);
    }
    
    socket = new QUdpSocket();
    socket->bind(8800);

    audio = new QAudioOutput(format, this);
    connect(audio, SIGNAL(stateChanged(QAudio::State)), this, SLOT(handleStateChanged(QAudio::State)));
    device = audio->start();
    connect(socket, SIGNAL(readyRead()), this, SLOT(playData()));
}

Player::~Player()
{
    device->close();
    socket->close();
    delete audio;
    delete device;
    delete socket;
}

void Player::handleStateChanged(QAudio::State newState)
{
    switch (newState) 
    {
        case QAudio::ActiveState:
            qDebug() << "Active: Player";
        break;
        case QAudio::IdleState:
            // Finished playing (no more data)
            qDebug() << "Finished: Player";
        break;

        case QAudio::StoppedState:
            // Stopped for other reasons
            if (audio->error() != QAudio::NoError) {
                // Error handling
                qWarning() << "Error found: Player";
            }
        break;

        default:
            // ... other cases as appropriate
        qDebug() << "Other";
        break;
    }
}

void Player::playData()
{
    while (socket->hasPendingDatagrams())
    {
        QByteArray data;
        data.resize(socket->pendingDatagramSize());
        socket->readDatagram(data.data(), data.size());
        device->write(data.data(), data.size());
    }
}
