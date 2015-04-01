/*------------------------------------------------------------------------------------------------------------------
-- SOURCE FILE: multicast.cpp
--
-- PROGRAM: CommAudio_Client
--
-- FUNCTIONS:
--
-- DATE: March 12, 2015
--
-- REVISIONS: Created March 10, 2015
--
-- DESIGNER: Michael Chimick
--
-- PROGRAMMER: Michael Chimick
--
-- NOTES:
--
--
----------------------------------------------------------------------------------------------------------------------*/

#define WIN32_LEAN_AND_MEAN
#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <mmsystem.h>
#include "music.h"
#include "socketinfo.h"
#include "client.h"
#include "multicast.h"

using std::cout;
using std::cerr;
using std::endl;

SOCKET_INFORMATION socketInfo;

//audio
CircularBuffer multiBuffer;
//media output device
HWAVEOUT multicastOutput;

//stream thread handle
HANDLE multiThread = INVALID_HANDLE_VALUE;
HANDLE multiParentThread = INVALID_HANDLE_VALUE;

bool streaming = false;

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: DropMulticast
--
-- DATE: March 12, 2015
--
-- REVISIONS: Created March 10, 2015
--
-- DESIGNER: Michael Chimick
--
-- PROGRAMMER: Michael Chimick
--
-- INTERFACE: void DropMulticast()
--
-- RETURNS: void
--
-- NOTES:
-- Drops the multicast session, and stops multicast processing
--
----------------------------------------------------------------------------------------------------------------------*/
bool StartMulticast(in_addr group)
{
	DWORD thread;


	multiParentThread = CreateThread(NULL, 0, JoinMulticast, (void *)&group, 0, &thread);

	if (multiParentThread == NULL)
	{
		cerr << "Multicast: Thread creation error (" << WSAGetLastError() << ")" << endl;
		return false;
	}

	GetExitCodeThread(multiParentThread, &thread);
	if (thread != STILL_ACTIVE)
	{
		cerr << "Multicast: Parent start error (" << WSAGetLastError() << ")" << endl;
		return false;
	}

	return true;
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: DropMulticast
--
-- DATE: March 12, 2015
--
-- REVISIONS: Created March 10, 2015
--
-- DESIGNER: Michael Chimick
--
-- PROGRAMMER: Michael Chimick
--
-- INTERFACE: void DropMulticast()
--
-- RETURNS: void
--
-- NOTES:
-- Drops the multicast session, and stops multicast processing
--
----------------------------------------------------------------------------------------------------------------------*/
bool EndMulticast()
{
	if (setsockopt(socketInfo.socket, IPPROTO_IP, IP_DROP_MEMBERSHIP, (char *)&(socketInfo.addr), sizeof(socketInfo.addr)) == SOCKET_ERROR)
	{
		cerr << "Multicast: drop membership error (" << WSAGetLastError() << ")" << endl;
		return false;
	}

	streaming = false;

	closesocket(socketInfo.socket);

	if (TerminateThread(multiThread, 0) == 0)
	{
		cerr << "Multicast: terminate thread error (" << WSAGetLastError() << ")" << endl;
	}

	if (TerminateThread(multiParentThread, 0) == 0)
	{
		cerr << "Multicast: terminate thread error (" << WSAGetLastError() << ")" << endl;
	}

	waveOutClose(multicastOutput); // attempting to open it again will fail if it isn't closed

	return true;
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: JoinMulticast
--
-- DATE: March 12, 2015
--
-- REVISIONS: Created March 10, 2015
--
-- DESIGNER: Michael Chimick
--
-- PROGRAMMER: Michael Chimick
--
-- INTERFACE: void JoinMulticast(SOCKET multicast, in_addr group)
--
-- RETURNS: void
--
-- NOTES:
-- Joins the multicast session, and starts multicast processing
--
----------------------------------------------------------------------------------------------------------------------*/
DWORD WINAPI JoinMulticast(LPVOID parameter)
{
	DWORD recvThread;

	BOOL flag = true;

	in_addr * group = (in_addr *)parameter;


	socketInfo.socket = socket(AF_INET, SOCK_DGRAM, 0);
	if (socketInfo.socket == INVALID_SOCKET)
	{
		cerr << "Multicast: Socket creation failure (" << WSAGetLastError() << ")" << endl;
		return -1;
	}

	if (setsockopt(socketInfo.socket, SOL_SOCKET, SO_REUSEADDR, (char *)&flag, sizeof(flag)) == SOCKET_ERROR)
	{
		cerr << "Multicast: Socket options set err (" << WSAGetLastError() << ")" << endl;
		return -1;
	}

	socketInfo.sockAddr.sin_family = AF_INET;
	socketInfo.sockAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	socketInfo.sockAddr.sin_port = htons(MULTICAST_PORT);
	if (bind(socketInfo.socket, (struct sockaddr*)&(socketInfo.sockAddr), sizeof(socketInfo.sockAddr)) == SOCKET_ERROR)
	{
		cerr << "Multicast: Bind error (" << WSAGetLastError() << ")" << endl;
		return -1;
	}

	//socketInfo.addr.imr_multiaddr = *group;
	//socketInfo.addr.imr_interface.S_un.S_addr = INADDR_ANY;
	socketInfo.addr.imr_multiaddr.s_addr = inet_addr("234.5.6.7");
	socketInfo.addr.imr_interface.s_addr = INADDR_ANY;

	if (setsockopt(socketInfo.socket, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *)&(socketInfo.addr), sizeof(socketInfo.addr)) == SOCKET_ERROR)
	{
		cerr << "Multicast: add membership error (" << WSAGetLastError() << ")" << endl;
		return -1;
	}


	//begin streaming audio
	DWORD threadId;
	if ((multiThread = CreateThread(NULL, 0, playMulticastSong, (LPVOID)multicastOutput, 0, &threadId)) == NULL)
	{
		cerr << "Unable to create unicast thread";
		return -1;
	}

	//begin reading data from the server
	receiveMulticastData();


	return 0;
}



void receiveMulticastData()
{
	streaming = true;

	while (streaming)
	{
		//receive data from the server
		int serverInfoSize = sizeof(socketInfo.sockAddr);
		int numReceived = 0;
		char tempBuffer[MESSAGE_SIZE];

		if ((numReceived = recvfrom(socketInfo.socket, tempBuffer, MESSAGE_SIZE,
			0, (struct sockaddr*) &socketInfo.sockAddr, &serverInfoSize)) < 0)
		{
			cerr << "Error reading data from multicast socket." << endl;
			continue;
		}
		cout << "Received:" << tempBuffer << endl;
		//place the data into the circular buffer
		for (int i = 0; i < numReceived; i++)
		{
			multiBuffer.buf[multiBuffer.pos] = tempBuffer[i];
			if (multiBuffer.pos == MUSIC_BUFFER_SIZE - 1)
			{
				multiBuffer.pos = 0;
			}
			else
			{
				multiBuffer.pos++;
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
DWORD WINAPI playMulticastSong(LPVOID arg)
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
	if (waveOutOpen(&multicastOutput, WAVE_MAPPER, &wavFormat, (DWORD)MultiWaveCallback, NULL, CALLBACK_FUNCTION) != MMSYSERR_NOERROR)
	{
		cerr << "Failed to open output device." << endl;
		return -1;
	}

	// Prepare the wave headers
	for (int i = 0; i < NUM_OUTPUT_BUFFERS; i++)
	{
		//malloc and clear the memory
		audioBuffers[i] = (LPWAVEHDR)malloc(sizeof(WAVEHDR));
		ZeroMemory(audioBuffers[i], sizeof(WAVEHDR));

		//update the their buffers to a position in the tripple buffer
		audioBuffers[i]->lpData = multiBuffer.buf;
		audioBuffers[i]->dwBufferLength = MUSIC_BUFFER_SIZE;

		// Create the header
		if (waveOutPrepareHeader(multicastOutput, audioBuffers[i], sizeof(WAVEHDR)) != MMSYSERR_NOERROR)
		{
			cerr << "Failed to create output header." << endl;
			exit(1);
		}
	}

	//write audio to the buffer
	cout << "I am ready to play music!" << endl;

	//wait until we have two messages worth of data; this avoids crackle
	while (multiBuffer.pos < MESSAGE_SIZE * 5)
	{
		//wait for the buffer to be ready
	}

	for (int i = 0; i < NUM_OUTPUT_BUFFERS; i++)
	{
		waveOutWrite(multicastOutput, audioBuffers[i], sizeof(WAVEHDR));
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
void CALLBACK MultiWaveCallback(HWAVEOUT hWave, UINT uMsg, DWORD dwUser, DWORD dw1, DWORD dw2)
{
	if (uMsg == WOM_DONE)
	{
		if (waveOutWrite(multicastOutput, (LPWAVEHDR)dw1, sizeof(WAVEHDR)) != MMSYSERR_NOERROR)
		{
			cerr << "Failed to play audio." << endl;
			exit(1);
		}
	}
}


