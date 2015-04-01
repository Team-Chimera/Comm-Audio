
#define WIN32_LEAN_AND_MEAN
#include <WinSock2.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <iostream>
#include <string>
#include <mmsystem.h>
#include "music.h"
#include "socketinfo.h"
#include "unicastSong.h"

using namespace std;

//UDP data socket
SOCKET unicastSongSocket;

HANDLE streamThread = INVALID_HANDLE_VALUE;

//flag for the song being finished
bool finished = false;

//media output device
HWAVEOUT output;

//server structure
sockaddr_in server;

//circular buffer
CircularBuffer circBuf;

/*******************************************************************
** Function: uncastSong
**
** Date: March 28th, 2015
**
** Revisions:
**
**
** Designer: Rhea Lauzon
**
** Programmer: Rhea Lauzon
**
** Interface:
**			DWORD WINAPI unicastSong(LPVOID host)
**              LPVOID host -- Host structure passed into the thread
**
** Returns:
**			DWORD -- -1 on failure of creating UDP socket,
**                      0 on success
**
** Notes:
** Creates the Unicast UDP socket and begins reading song data.
**
*******************************************************************/
DWORD WINAPI unicastSong(LPVOID host)
{
    //initialize the buffer position to 0
    circBuf.pos = 0;

    //conver tthe parameter
    hostent *hp = (hostent *) host;

    //set up the structure
    server.sin_family = AF_INET;
    server.sin_port = htons(CLIENT_UNICAST_PORT);
    server.sin_addr.s_addr = htonl(INADDR_ANY);


    // Copy the server address from the resolved host
    memcpy((char *)&server.sin_addr, hp->h_addr, hp->h_length);

    //create the UDP socket
    if ((unicastSongSocket = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET)
    {
        cerr << "Unable to creat the Unicast UDP socket." << endl;
        return -1;
    }

    //bind the socket
    if (bind(unicastSongSocket, (struct sockaddr *) &server, sizeof(server)) == -1)
    {
        // The socket failed to be bound
        cerr << "Failed to bind unicast socket." << endl;
        WSACleanup();
        exit(1);
    }


    //set to re-use address state
    BOOL flag = true;
    if (setsockopt(unicastSongSocket, SOL_SOCKET, SO_REUSEADDR, (char *)&flag, sizeof(flag)) == SOCKET_ERROR)
    {
        cerr << "Unable to set unicast socket to reusable" << endl;
        return -1;
    }

    //begin streaming audio
    DWORD threadId;
    if ((streamThread = CreateThread(NULL, 0, playSong, (LPVOID) output, 0, &threadId)) == NULL)
    {
        cerr << "Unable to create unicast thread";
        return false;
    }

    //begin reading data from the server
    receiveData();


    return 0;
}


void receiveData()
{
    while(!finished)
    {
        //receive data from the server
       int serverInfoSize = sizeof(server);
       int numReceived = 0;
       char tempBuffer[MESSAGE_SIZE];

       if ((numReceived = recvfrom(unicastSongSocket, tempBuffer, MESSAGE_SIZE,
       0, (struct sockaddr*) &server, &serverInfoSize)) < 0)
       {
           cerr << "Error reading data from unicast socket." << endl;
           continue;
       }
       //place the data into the circular buffer
       for (int i = 0; i < numReceived; i++)
       {
           circBuf.buf[circBuf.pos] = tempBuffer[i];
           if(circBuf.pos == MUSIC_BUFFER_SIZE - 1)
           {
               circBuf.pos = 0;
           }
           else
           {
               circBuf.pos++;
           }
       }

    }
}

/*****************************************************************
** Function: playSong
**
** Date: March 28th, 2015
**
** Revisions:
**
**
** Designer: Rhea Lauzon
**
** Programmer: Rhea Lauzon
**
** Interface:
**			DWORD WINAPI playSong(LPVOID arg)
**				LPVOID arg -- Thread arguments
**
** Returns:
**          DWORD -- -1 on failure; 0 on success
**
** Notes:
** Plays the received song in a thread.
**
*******************************************************************/
DWORD WINAPI playSong(LPVOID arg)
{
    //create the wave header
    WAVEFORMATEX wavFormat;

    //create a number of buffers; in this case 3
    LPWAVEHDR audioBuffers[NUM_OUTPUT_BUFFERS];

    //set up the format
    wavFormat.nSamplesPerSec = 44100;
    wavFormat.wBitsPerSample = 16;
    wavFormat.nChannels = 2;
    wavFormat.cbSize = 0;
    wavFormat.wFormatTag = WAVE_FORMAT_PCM;
    wavFormat.nBlockAlign = wavFormat.nChannels * (wavFormat.wBitsPerSample / 8);
    wavFormat.nAvgBytesPerSec = wavFormat.nSamplesPerSec * wavFormat.wBitsPerSample;

    //open the media device for streaming to
    if (waveOutOpen(&output, WAVE_MAPPER, &wavFormat, (DWORD) waveCallback, NULL, CALLBACK_FUNCTION) != MMSYSERR_NOERROR)
    {
        cerr << "Failed to open output device." << endl;
        return -1;
    }

    // Prepare the wave headers
    for (int i = 0; i < NUM_OUTPUT_BUFFERS; i++)
    {
        //malloc and clear the memory
        audioBuffers[i] = (LPWAVEHDR) malloc(sizeof(WAVEHDR));
        ZeroMemory(audioBuffers[i], sizeof(WAVEHDR));

        //update the their buffers to a position in the tripple buffer
        audioBuffers[i]->lpData = circBuf.buf;
        audioBuffers[i]->dwBufferLength = MUSIC_BUFFER_SIZE;

        // Create the header
        if (waveOutPrepareHeader(output, audioBuffers[i], sizeof(WAVEHDR)) != MMSYSERR_NOERROR)
        {
            cerr << "Failed to create output header." << endl;
            exit(1);
        }
    }

    //write audio to the buffer
    cout << "I am ready to play music!" << endl;

    //wait until we have two messages worth of data; this avoids crackle
    while(circBuf.pos < MESSAGE_SIZE * 5)
    {
        //wait for the buffer to be ready
    }

    for (int i = 0; i < NUM_OUTPUT_BUFFERS; i++)
    {
        waveOutWrite(output, audioBuffers[i], sizeof(WAVEHDR));
    }

    return 0;

}

/*****************************************************************
** Function: waveCallback
**
** Date: March 28th, 2015
**
** Revisions:
**
**
** Designer: Rhea Lauzon
**
** Programmer: Rhea Lauzon
**
** Interface:
**			void CALLBACK waveCallback(HWAVEOUT hWave, UINT uMsg,
**								DWORD dwUser, DWORD dw1, DWORD dw2)
**				HWAVEOUT hWave -- handle to the output device
**				UINT uMsg -- message sent to the callback
**				DWORD dwUser -- Unused parameter
**				DWORD dw1 -- the header used for this audio output
**				DWORD dw2 -- Unused parameter
**
**
** Returns:
**          void
**
** Notes:
** Plays the audio without skipping issues.
**
*******************************************************************/
void CALLBACK waveCallback(HWAVEOUT hWave, UINT uMsg, DWORD dwUser, DWORD dw1, DWORD dw2)
{
    if (uMsg == WOM_DONE)
    {
        if (waveOutWrite(output, (LPWAVEHDR) dw1, sizeof(WAVEHDR)) != MMSYSERR_NOERROR)
        {
            cerr << "Failed to play audio." << endl;
            exit(1);
        }
    }
}



