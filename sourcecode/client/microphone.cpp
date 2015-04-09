/**********************************************************************
 *
**	SOURCE FILE:	microphone.cpp -  Microphone class for voice chat
**
**	PROGRAM:	Comm Audio
**
**	FUNCTIONS:
**         Microphone();
**         ~Microphone();
**         void startVoice(QString);
**         void stopVoice();
**         void handleStateChanged(QAudio::State newState);
**
**
**	DATE: 		April 1, 2015
**
**
**	DESIGNER:	Julian Brandrick
**
**
**	PROGRAMMER: Julian Brandrick
**
**	NOTES:
**  Records audio from the default input device and sends it to through a UDP
**   socket.
*************************************************************************/

#include <QIODevice>
#include <QAudioFormat>
#include <QAudioDeviceInfo>
#include <QTimer>
#include <QDebug>
#include <QObject>
#include "microphone.h"

/*------------------------------------------------------------------------------
-- FUNCTION: Microphone
--
-- DATE: April 1, 2015
--
-- DESIGNER: Julian Brandrick
--
-- PROGRAMMER: Julian Brandrick
--
-- INTERFACE: Microphone(QString address)
--
-- PARAMETER:
--      address - IP address for the ptp client.
--
-- NOTES:
--  Writes the recording audio directly to the created UDP socket.
------------------------------------------------------------------------------*/
Microphone::Microphone() : QObject()
{
    // Sets up a format for recording the audio
    QAudioFormat format;
    format.setSampleRate(8000); 
    format.setChannelCount(1);
    format.setSampleSize(8);
    format.setCodec("audio/pcm");
    format.setByteOrder(QAudioFormat::LittleEndian);
    format.setSampleType(QAudioFormat::UnSignedInt);
    
    // Checks to see if the default recording device can handle the above 
    //  format. If not then it finds the nearest format it can use.
    QAudioDeviceInfo info = QAudioDeviceInfo::defaultInputDevice();
    if(!info.isFormatSupported(format))
    {
        qWarning() << "Default format not supported, finding next best...";
        format = info.nearestFormat(format);
    }
    
    // Sets up a QT UDP socket and gets it ready
    socket = new QUdpSocket();
    
    // Gets the recording device ready and connects the microphones state to 
    //  the handleStateChanged function.
    audio = new QAudioInput(format, this);
    connect(audio, SIGNAL(stateChanged(QAudio::State)), this, SLOT(handleStateChanged(QAudio::State)));
    
}

/*------------------------------------------------------------------------------
-- FUNCTION: ~Microphone
--
-- DATE: April 1, 2015
--
-- DESIGNER: Julian Brandrick
--
-- PROGRAMMER: Julian Brandrick
--
-- INTERFACE: ~Microphone()
--
-- NOTES:
--  Stops recording, closes the socket and frees their memory.
------------------------------------------------------------------------------*/
Microphone::~Microphone()
{
    audio->stop();
    socket->close();
    delete audio;
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
--      newState - The state of the microphone.
--
-- NOTES:
--  Gets the current state of the microphone.
------------------------------------------------------------------------------*/
void Microphone::handleStateChanged(QAudio::State newState)
{
    switch(newState)
    {
        case QAudio::StoppedState:
            if(audio->error() != QAudio::NoError)
            {
                // Error handling
                qWarning() << "Microphone Error: " << audio->error();
                
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
            // other cases
            qDebug() << "Something weird";
        break;
    }
}


/*------------------------------------------------------------------------------
-- FUNCTION: startVoice
--
-- DATE: April 2, 2015
--
-- DESIGNER: Julian Brandrick
--
-- PROGRAMMER: Rhea lauzon based on Julian`s code
--
-- INTERFACE: void startVoice(QString address)
--
-- PARAMETER:
--      address -- Address to send to
--
-- NOTES:
--  Begins the voice chat
------------------------------------------------------------------------------*/
void Microphone::startVoice(QString address)
{
    socket->disconnectFromHost();
    socket->connectToHost(address, VOICE_CHAT_CHANNEL);

    // Starts the microphone recording and writes it directly to the socket.
    audio->start(socket);
}

/*------------------------------------------------------------------------------
-- FUNCTION: stopVoice
--
-- DATE: April 2, 2015
--
-- DESIGNER: Julian Brandrick
--
-- PROGRAMMER: Rhea lauzon based on Julian`s code
--
-- INTERFACE: void stopVoice()
--
-- PARAMETER:
--      none
--
-- NOTES:
--  Stops the voice chat
------------------------------------------------------------------------------*/
void Microphone::stopVoice()
{
    audio->stop();
}
