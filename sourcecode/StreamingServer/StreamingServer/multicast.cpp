#include "stdafx.h"
#include <iostream>
#include "multicast.h"
#include "helper.h"
#include <mmsystem.h>
#include <vlc/vlc.h>
#include <vlc/libvlc.h>
#include "music.h"

#pragma comment(lib, "libvlc.lib")
#pragma comment(lib, "libvlccore.lib")

//VLC instance objects
libvlc_instance_t *inst;
libvlc_media_player_t *mediaPlayer;


//circular buffer
CircularBuffer circBuf;

//multicast info
 LPMULTICAST_INFORMATION lpMulticastInfo;

using namespace std;

/*******************************************************************
** Function: startMulticastThread
**
** Date: March 12th, 2015
**
** Revisions:
**
**
** Designer: Julian Brandrick
**
** Programmer: Julian Brandrick
**
** Interface:
**			DWORD WINAPI startMulticastThread(LPVOID lpParameter)
**
** Parameters:
**          multicastThread - Pointer to a thread handle
**
** Returns:
**			true on success
**          false if error occurs
**
** Notes:
**  The main function for the multicast session. It initializes the 
**  multicast structure and starts sending in a separate thread.
*******************************************************************/

DWORD WINAPI startMulticastThread(LPVOID lpParameter)
{
    char multicastAddr[16] = TIMECAST_ADDR;
    WORD wVersionRequested = MAKEWORD (2,2);
    WSAData wsaData;
    int loop;
    u_char lTTL;

   // cout << "Id of multicast thread " << GetCurrentThreadId() << endl;

    WSAStartup(wVersionRequested, &wsaData);

    if((lpMulticastInfo = initMulticastSocket()) == NULL)
    {
        return false;
    }

    lpMulticastInfo->StMreq.imr_multiaddr.s_addr = inet_addr(multicastAddr);
    lpMulticastInfo->StMreq.imr_interface.s_addr = INADDR_ANY;
    if(setsockopt(lpMulticastInfo->Socket, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *)&lpMulticastInfo->StMreq, sizeof(lpMulticastInfo->StMreq)) == SOCKET_ERROR)
    {
        displayError("setsockopt IP_ADD_MEMBERSHIP failed", WSAGetLastError());
        return false;
    }
    
    lTTL = TIMECAST_TTL;
    if(setsockopt(lpMulticastInfo->Socket, IPPROTO_IP, IP_MULTICAST_TTL, (char *)&lTTL, sizeof(lTTL)) == SOCKET_ERROR)
    {
        displayError("setsockopt IP_MULTICAST_TTL failed", WSAGetLastError());
        return false;
    }

    loop = 0;
    if(setsockopt(lpMulticastInfo->Socket, IPPROTO_IP, IP_MULTICAST_LOOP, (char *)&loop, sizeof(loop)) == SOCKET_ERROR)
    {
        displayError("setsockopt IP_MULTICAST_LOOP failed", WSAGetLastError());
        return false;
    }

	/** Make the music! **/
	char memoryOptions[256];
	sprintf_s(memoryOptions, VLC_OPTIONS, (long long int)(intptr_t)(void*) &handleStream, (long long int)(intptr_t)(void*) &prepareRender);
	const char* const vlcArgs[] = { "-I", "dummy", "--verbose=0", "--sout", memoryOptions};
	
	 //create an instance of libvlc
	if ((inst = libvlc_new(sizeof(vlcArgs) / sizeof(vlcArgs[0]), vlcArgs)) == NULL)
	{
		cerr << "Failed to create libvlc instance." << endl;
		return false;
	}

	//load a song from command line else the test song
<<<<<<< HEAD
	libvlc_media_t *song = libvlc_media_new_path(inst, "test.m4a");
=======
   // std::string song_to_play(song_dir + "test.mp3");
	libvlc_media_t *song = libvlc_media_new_path(inst, "test.mp3");
>>>>>>> a7ab8e3f588101060e3a6fc4a4f1b56a9f9d3da8

	//load the song
	if (song == NULL)
	{
		cerr << "failed to load the song." << endl;
		return false;
	}

	//create a media player
	mediaPlayer = libvlc_media_player_new_from_media(song);

	//Begin playing the music
	libvlc_media_release(song);

	//starts the process of streaming the audio (calls pre-render then handleStream)
	libvlc_media_player_play(mediaPlayer);

    return 0;
}

/*******************************************************************
** Function: multicastSendLoop
**
** Date: March 12th, 2015
**
** Revisions:
**
**
** Designer: Julian Brandrick
**
** Programmer: Julian Brandrick
**
** Interface:
**			DWORD WINAPI multicastSendLoop(LPVOID lpParam)
**
** Parameters:
**          lpParam - Contains a Multicast structure
**
** Returns:
**			FALSE upon failure
**
** Notes:
**  This is the thread routine for the multicast send loop. It
**  sends data over a multicast channel using a completion routine.
*******************************************************************/
DWORD WINAPI multicastSendLoop(LPVOID lpParam)
{
    LPMULTICAST_INFORMATION lpMulticastInfo = (LPMULTICAST_INFORMATION)lpParam;

    int index;
    DWORD flags;
    DWORD sendBytes;
    WSAEVENT eventArray[1];

    strcpy_s(lpMulticastInfo->Buffer, "This is from a Completion Port UDP Server");

    flags = 0;
    
    while(TRUE)
    {
        if (WSASendTo(lpMulticastInfo->Socket, &(lpMulticastInfo->DataBuf), 1, &sendBytes, flags, (sockaddr *)&(lpMulticastInfo->Client), sizeof(lpMulticastInfo->Client), &(lpMulticastInfo->Overlapped), multicastRoutine) == SOCKET_ERROR)
        {
            if (WSAGetLastError() != WSA_IO_PENDING)
            {
                displayError("WSASendTo failed", WSAGetLastError()); 
                return -1;
            }
        }

        eventArray[0] = WSACreateEvent(); 
        while(TRUE)
        {
            index = WSAWaitForMultipleEvents(1, eventArray, FALSE, WSA_INFINITE, TRUE);

            if (index == WAIT_IO_COMPLETION)
            {
                break;
            }
            else
            {
                displayError("WSAWaitForMultipleEvents failed", WSAGetLastError());
                return -1;
            }
        }
    }

    return FALSE;
}

/*******************************************************************
** Function: multicastRoutine
**
** Date: March 12th, 2015
**
** Revisions:
**
**
** Designer: Julian Brandrick
**
** Programmer: Julian Brandrick
**
** Interface:
**			void CALLBACK multicastRoutine(DWORD Error, 
**                                         DWORD BytesTransferred, 
**                                         LPWSAOVERLAPPED Overlapped, 
**                                         DWORD InFlags)
**
** Parameters:
**          Error - Contains any errors that occur
**          BytesTransferred - Number of bytes transferred
**          Overlapped - Pointer to an overlapped struct
**          InFlags - Flags used for sending
**
** Returns:
**			FALSE upon failure
**
** Notes:
**  This is the completion routine that handles sending data to the
**  multicast clients.
*******************************************************************/
void CALLBACK multicastRoutine(DWORD Error, DWORD BytesTransferred, LPWSAOVERLAPPED Overlapped, DWORD InFlags)
{
    DWORD sendBytes;
    DWORD flags;
    int clientLen;

    LPMULTICAST_INFORMATION MI = (LPMULTICAST_INFORMATION) Overlapped;

    if (Error != 0 || BytesTransferred == 0)
    {
        displayError("Routine error", Error);
        closesocket(MI->Socket);
        GlobalFree(MI);
        return;
    }

    MI->BytesSent += BytesTransferred;
    flags = 0;
    ZeroMemory(Overlapped, sizeof(WSAOVERLAPPED));

    MI->DataBuf.len = DATA_BUFSIZE;
    MI->DataBuf.buf = MI->Buffer;

    clientLen = sizeof(MI->Client);

    if (WSASendTo(MI->Socket, &(MI->DataBuf), 1, &sendBytes, flags, (sockaddr *)&(MI->Client), clientLen, &(MI->Overlapped), multicastRoutine) == SOCKET_ERROR)
    {
        if (WSAGetLastError() != WSA_IO_PENDING)
        {
            displayError("WSASendTo failed", WSAGetLastError());
            return;
        }
    }

    printf("\nRoutine: %s\n", MI->Buffer);

    Sleep(TIMECAST_INTRVL * 1000);
}

/*******************************************************************
** Function: initMulticastSocket
**
** Date: March 12th, 2015
**
** Revisions:
**
**
** Designer: Julian Brandrick
**
** Programmer: Julian Brandrick
**
** Interface:
**			LPMULTICAST_INFORMATION initMulticastSocket()
**
** Returns:
**			- An initialized multicast structure upon success
**          - NULL upon failure
**
** Notes:
**  This function completely initializes a multicast structure.
*******************************************************************/
LPMULTICAST_INFORMATION initMulticastSocket()
{
    LPMULTICAST_INFORMATION lpMulticastInfo;
    SOCKADDR_IN server;

    if ((lpMulticastInfo = (LPMULTICAST_INFORMATION) GlobalAlloc(GPTR, sizeof(MULTICAST_INFORMATION))) == NULL)
    {
        displayError("GlobalAlloc failed", GetLastError());
        return NULL;
    }

    if ((lpMulticastInfo->Socket = WSASocket(PF_INET, SOCK_DGRAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED)) == INVALID_SOCKET)
    {
        displayError("Socket error", WSAGetLastError());
        return NULL;
    }

    memset ((char *)&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = 0;
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    
    lpMulticastInfo->Client.sin_family      = AF_INET;
    lpMulticastInfo->Client.sin_addr.s_addr = inet_addr(TIMECAST_ADDR);
    lpMulticastInfo->Client.sin_port        = htons(TIMECAST_PORT);

    if ( (lpMulticastInfo->Socket, (struct sockaddr *)&server, sizeof(server)) == -1)
    {
        displayError("Bind error", WSAGetLastError());
        return NULL;
    }

    ZeroMemory(&(lpMulticastInfo->Overlapped), sizeof(WSAOVERLAPPED));
    lpMulticastInfo->BytesSent = 0;
    lpMulticastInfo->DataBuf.len = DATA_BUFSIZE;
    lpMulticastInfo->DataBuf.buf = lpMulticastInfo->Buffer;

    return lpMulticastInfo;
}

/*******************************************************************
** Function: displayError
**
** Date: March 12th, 2015
**
** Revisions:
**
**
** Designer: Julian Brandrick
**
** Programmer: Julian Brandrick
**
** Interface:
**			void displayError(char *errStr, int errCode)
** 
** Parameters:
**          errStr - Error string to be displayed
**          errCode - Error code that was thrown
**
** Returns:
**			void
**
** Notes:
**  This function prints a formatted error string and code to 
**  stderr.
*******************************************************************/
void displayError(char *errStr, int errCode)
{
    fprintf(stderr, "%s: %d\n", errStr, errCode);
}

/*****************************************************************
** Function: audioCleanup
**
** Date: March 30th, 2015
**
** Revisions:
**
**
** Designer: Rhea Lauzon
**
** Programmer: Rhea Lauzon
**
** Interface:
**			void audioCleanup()
**
** Returns:
**          void
**
** Notes:
** Removes the libVLC structures for sending data.
**
*******************************************************************/
void audioCleanup()
{
	libvlc_media_player_release(mediaPlayer);
    libvlc_release(inst);

}

/*****************************************************************
** Function: handleStream
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
**			void handleStream(void* p_audio_data, uint8_t* p_pcm_buffer, 
**			unsigned int channels, unsigned int rate, unsigned int nb_samples,
**			unsigned int bits_per_sample, size_t size, int64_t pts)
**				void * p_audio_data -- Unused parameter
**				uint8_t * p_pcm_buffer -- pointer to location of the buffer
**				unsigned int channels -- Number of channels in use
**				unsigned int rate -- Bitrate
**				unsigned int nb_samples -- Samples/second of audio
**				unsigned int bits_per_sample -- Number of bits/sample (depth-wise)
**				size_t size -- Length of the buffer
**				int64_t pts -- Unused parameter
**				
**
** Returns:
**          void
**
** Notes:
** Handler for when the audio is streamed to write data
**
*******************************************************************/
void handleStream(void* p_audio_data, uint8_t* p_pcm_buffer, unsigned int channels, 
				  unsigned int rate, unsigned int nb_samples, unsigned int bits_per_sample, size_t size, int64_t pts)
{
	char *buffer;
	int dataSize = size;
	int messageSize;
	int dataSent = 0;

 

	// While we have data to write
	while (dataSize > 0)
	{
		// Set the size of the next message to send
		if (dataSize > MESSAGE_SIZE)
		{
			messageSize = MESSAGE_SIZE;
		}
		else
		{
			messageSize = dataSize;
		}

		// Write the data to the circular buffer
		buffer = new char[dataSize];
		memcpy(buffer, p_pcm_buffer + dataSent, messageSize);
		
		//place the data into the circular buffer		
		for (int i = 0; i < messageSize; i++)
		{
			circBuf.buf[circBuf.pos] = buffer[i];
			if(circBuf.pos == BUFFER_SIZE - 1)
			{
				circBuf.pos = 0;
			}
			else
			{
				circBuf.pos++;
			}
		}
		
		//update the sizing
		dataSize -= messageSize;
		dataSent += messageSize;

		
		//send over UDP
		if(sendto(lpMulticastInfo->Socket, buffer, MESSAGE_SIZE, 0,(struct sockaddr *)&(lpMulticastInfo->Client), sizeof(lpMulticastInfo->Client)) <= 0)
		{
			perror ("sendto error");
			closesocket(lpMulticastInfo->Socket);
			WSACleanup();
			exit(0);
		}
        //   cout << "multicast sent! thread ID:  " << GetCurrentThreadId() << endl;
		

		//remove the char array
		delete [] buffer;

	}

	// Free the temporary stream buffer
	free(p_pcm_buffer);
}


/*****************************************************************
** Function: prepareRender
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
**			void prepareRender(void* p_audio_data, 
**			uint8_t** pp_pcm_buffer , size_t size)
**				void * p_audio_data -- unusued parameter
**				uint8_t ** pp_pcm_buffer -- Location of the audio stream
**				size_t size -- Size of the buffer
**
**				
**
** Returns:
**          void
**
** Notes:
** Called before audio streaming to allocate memory for the buffer
**
*******************************************************************/
void prepareRender(void* p_audio_data, uint8_t** pp_pcm_buffer , size_t size)
{

  //  cout << "Id of prepare renderer " << GetCurrentThreadId() << endl;
	// Allocate memory to the buffer
	*pp_pcm_buffer = (uint8_t*) malloc(size);
}
