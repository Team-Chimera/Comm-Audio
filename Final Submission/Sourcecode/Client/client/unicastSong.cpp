/**********************************************************************
 *
**	SOURCE FILE:	unicastdialog.cpp -  Custom dialog class for the unicast selections
**
**	PROGRAM:	Comm Audio
**
**	FUNCTIONS:
**         DWORD WINAPI unicastSong(LPVOID);
**         void receiveData();
**
**         //function prototpes
**         DWORD WINAPI unicastSong(LPVOID arg);
**         void CALLBACK waveCallback(HWAVEOUT hWave, UINT uMsg, DWORD dwUser, DWORD dw1, DWORD dw2);
**         void prepareRender(void* p_audio_data, uint8_t** pp_pcm_buffer , size_t size);
**         void handleStream(void* p_audio_data, uint8_t* p_pcm_buffer, unsigned int channels,
**                           unsigned int rate, unsigned int nb_samples, unsigned int bits_per_sample, size_t size, int64_t pts);
**         void audioCleanup();
**         void setUniVolume(int);
**         int endUnicast();
**
**
**
**	DATE: 		March 27th, 2015
**
**
**	DESIGNER:	Rhea Lauzon A00881688
**
**
**	PROGRAMMER: Rhea Lauzon A00881688
**
**	NOTES:
**	Dialog class for unicast menu for songs
*************************************************************************/
#define WIN32_LEAN_AND_MEAN
#include <WinSock2.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <iostream>
#include <string>
#include <mmsystem.h>
#include "multicast.h"
#include "music.h"
#include "socketinfo.h"
#include "unicastSong.h"

using namespace std;

//UDP data socket
SOCKET unicastSongSocket;

HANDLE streamThread = INVALID_HANDLE_VALUE;

//flag for the song being finished
bool finished = false;


//server structure
sockaddr_in unicastInfo;

//circular buffer
CircularBuffer * uniCircBuf;

int uniVolume = 50;

/*******************************************************************
** Function: unicastSong
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
    finished = false;

    uniCircBuf = getCircularBuffer();
    //initialize the buffer position to 0
    uniCircBuf->pos = 0;

    //create the UDP socket
    if ((unicastSongSocket = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET)
    {
        cerr << "Unable to creat the Unicast UDP socket." << endl;
        return -1;
    }

    //bind the socket to the specified address
    unicastInfo.sin_family = AF_INET;
    unicastInfo.sin_port = htons(CLIENT_UNICAST_PORT);
    unicastInfo.sin_addr.s_addr = htonl(INADDR_ANY);

    //bind the socket
    if (bind(unicastSongSocket, (struct sockaddr *) &unicastInfo, sizeof(unicastInfo)) == -1)
    {
        // The socket failed to be bound
        cerr << "Failed to bind unicast socket. Error " << WSAGetLastError() << endl;
        WSACleanup();
        return -1;
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
    if ((streamThread = CreateThread(NULL, 0, playMulticastSong, NULL, 0, &threadId)) == NULL)
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
       int size = sizeof(unicastInfo);
       int numReceived = 0;
       char tempBuffer[MESSAGE_SIZE];

       if ((numReceived = recvfrom(unicastSongSocket, tempBuffer, MESSAGE_SIZE,
       0, (struct sockaddr*) &unicastInfo, &size)) < 0)
       {
           cerr << "Error reading data from unicast socket." << endl;
           continue;
       }
       //place the data into the circular buffer
       for (int i = 0; i < numReceived; i++)
       {
           uniCircBuf->buf[uniCircBuf->pos] = tempBuffer[i] * uniVolume / 100;
           if(uniCircBuf->pos == MUSIC_BUFFER_SIZE - 1)
           {
               uniCircBuf->pos = 0;
           }
           else
           {
               uniCircBuf->pos++;
           }
       }

    }
}


/*******************************************************************
** Function: setUniVoulume
**
** Date: April 6th, 2015
**
** Revisions:
**
**
** Designer: Rhea Lauzon
**
** Programmer: Rhea Lauzon
**
** Interface:
**			void setUniVolume(int val)
**              int val -- new volume value
**
** Returns:
**			void
**
** Notes:
** Updates the volume of the song
**
*******************************************************************/
void setUniVolume(int val)
{
    uniVolume = val;
}



/*******************************************************************
** Function: endUnicast
**
** Date: April 6th, 2015
**
** Revisions:
**
**
** Designer: Rhea Lauzon
**
** Programmer: Rhea Lauzon
**
** Interface:
**			void endUnicast()
**
**
** Returns:
**			void
**
** Notes:
** Ends the unicast session
**
*******************************************************************/
int endUnicast()
{

    finished = true;

    //close the socket
    closesocket(unicastSongSocket);

    //end streaming thread
    if (TerminateThread(streamThread, 0) == 0)
    {
        cerr << "Unicast: terminate thread error (" << WSAGetLastError() << ")" << endl;
        return -1;
    }

    return 0;

}
