#include <QAudioFormat>
#include <QAudioDeviceInfo>
#include <QIODevice>
#include <QByteArray>
#include <QDebug>
#include "player.h"

/*------------------------------------------------------------------------------
-- FUNCTION: Player
--
-- DATE: April 1, 2015
--
-- DESIGNER: Julian Brandrick
--
-- PROGRAMMER: Julian Brandrick
--
-- INTERFACE: Player()
--
-- NOTES:
--  Reads the recorded audio from the socket and plays it for the user.
------------------------------------------------------------------------------*/
Player::Player() : QObject()
{
    // Sets up a format for playing audio
    QAudioFormat format;
    format.setSampleRate(8000);
    format.setChannelCount(1);
    format.setSampleSize(8);
    format.setCodec("audio/pcm");
    format.setByteOrder(QAudioFormat::LittleEndian);
    format.setSampleType(QAudioFormat::UnSignedInt);
    
    // Checks to see if the default recording device can handle the above 
    //  format. If not then it won't be able to play the audio.
    QAudioDeviceInfo info(QAudioDeviceInfo::defaultOutputDevice());
    if (!info.isFormatSupported(format)) 
    {
        qWarning() << "Raw audio format not supported by backend, cannot play audio.";
        return;
    }
    
    // Sets up a QT UDP socket and binds it to port 8800.
    socket = new QUdpSocket();
    socket->bind(VOICE_CHAT_CHANNEL);

    // Gets the playing device ready and connects the stereos state to the 
    //  handleStateChanged function and the socket to the playData function.
    audio = new QAudioOutput(format, this);
    connect(audio, SIGNAL(stateChanged(QAudio::State)), this, SLOT(handleStateChanged(QAudio::State)));
    connect(socket, SIGNAL(readyRead()), this, SLOT(playData()));
}

/*------------------------------------------------------------------------------
-- FUNCTION: ~Player
--
-- DATE: April 1, 2015
--
-- DESIGNER: Julian Brandrick
--
-- PROGRAMMER: Julian Brandrick
--
-- INTERFACE: ~Player()
--
-- NOTES:
--  Stops playing, closes the socket, closes the device and frees their memory.
------------------------------------------------------------------------------*/
Player::~Player()
{
    audio->stop();
    device->close();
    socket->close();
    delete audio;
    delete device;
    delete socket;
}

/*------------------------------------------------------------------------------
-- FUNCTION: handleStateChanged
--
-- DATE: April 1, 2015
--
-- DESIGNER: Julian Brandrick
--
-- PROGRAMMER: Julian Brandrick
--
-- INTERFACE: void handleStateChanged(QAudio::State newState)
--
-- PARAMETER:
--      newState - The state of the stereo.
--
-- NOTES:
--  Gets the current state of the stereo.
------------------------------------------------------------------------------*/
void Player::handleStateChanged(QAudio::State newState)
{
    switch(newState) 
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
            if (audio->error() != QAudio::NoError) 
            {
                // Error handling
                qWarning() << "Player Error: " << audio->error();
            }
        break;

        default:
            // other cases
            qDebug() << "Something weird";
        break;
    }
}

/*------------------------------------------------------------------------------
-- FUNCTION: playData
--
-- DATE: April 1, 2015
--
-- DESIGNER: Julian Brandrick
--
-- PROGRAMMER: Julian Brandrick
--
-- INTERFACE: void playData()
--
-- NOTES:
--  Reads the recorded audio data from the socket and writes it to the stereo.
------------------------------------------------------------------------------*/
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

void Player::startVoicePlay()
{
    device = audio->start();
}

void Player::stopVoicePlay()
{
    audio->stop();
}
