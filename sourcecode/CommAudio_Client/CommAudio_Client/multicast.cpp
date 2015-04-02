/*-------------------------------------------------------------------------------------------------
-- SOURCE FILE: mulitcast.cpp
--
-- PROGRAM: CommAudio_Client
--
-- FUNCTIONS:
--            //
--
-- DATE: April 1, 2015
--
-- DESIGNER: Michael Chimick
--
-- PROGRAMMER: Michael Chimick
--
-- NOTES:
--    //
-------------------------------------------------------------------------------------------------*/

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

//create a number of buffers; in this case 3
LPWAVEHDR audioBuffers[NUM_OUTPUT_BUFFERS];

//stream thread handle
HANDLE multiThread = INVALID_HANDLE_VALUE;
HANDLE multiParentThread = INVALID_HANDLE_VALUE;

bool streaming = false;

/******************************************************************************************************************************************
*******************************************************************************************************************************************
*
* START AND END FUNCTIONS
*
*******************************************************************************************************************************************
******************************************************************************************************************************************/

/*-------------------------------------------------------------------------------------------------
-- FUNCTION: StartMulticast
--
-- DATE: April 1, 2015
--
-- DESIGNER: Michael Chimick
--
-- PROGRAMMER: Michael Chimick
--
-- INTERFACE: bool StartMulticast()
--
-- RETURNS: bool // returns true is all calls successful, false otherwise
--
-- NOTES:
--    Starts receiving and playback threads for the multicasting data
-------------------------------------------------------------------------------------------------*/
bool StartMulticast()
{
	DWORD thread;

	multiParentThread = CreateThread(NULL, 0, JoinMulticast, NULL, 0, &thread);
	if (multiParentThread == NULL)
	{
		cerr << "Multicast: thread creation error (" << WSAGetLastError() << ")" << endl;
		return false;
	}

	GetExitCodeThread(multiParentThread, &thread);
	if (thread != STILL_ACTIVE)
	{
		cerr << "Multicast: parent start error (" << WSAGetLastError() << ")" << endl;
		return false;
	}

	return true;
}

/*-------------------------------------------------------------------------------------------------
-- FUNCTION: EndMulticast
--
-- DATE: April 1, 2015
--
-- DESIGNER: Michael Chimick
--
-- PROGRAMMER: Michael Chimick
--
-- INTERFACE: bool EndMulticast()
--
-- RETURNS: bool // returns true is all calls successful, false otherwise
--
-- NOTES:
--    Stops the receiving and playback threads, and resets variables
-------------------------------------------------------------------------------------------------*/
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

	// Unprepare the wave headers
	for (int i = 0; i < NUM_OUTPUT_BUFFERS; i++)
	{
		if (waveOutPrepareHeader(multicastOutput, audioBuffers[i], sizeof(WAVEHDR)) != MMSYSERR_NOERROR)
		{
			cerr << "Multicast: failed to unprepare output header" << endl;
			exit(1);
		}

		delete audioBuffers[i];
	}

	// Close the device
	waveOutClose(multicastOutput);

	return true;
}

/******************************************************************************************************************************************
*******************************************************************************************************************************************
*
* RECEIVE AND PLAYBACK FUNCTIONS
*
*******************************************************************************************************************************************
******************************************************************************************************************************************/

/*-------------------------------------------------------------------------------------------------
-- FUNCTION: JoinMulticast
--
-- DATE: April 1, 2015
--
-- DESIGNER: Michael Chimick
--
-- PROGRAMMER: Michael Chimick, Rhea Lauzon
--
-- INTERFACE: DWORD WINAPI JoinMulticast(LPVOID parameter)
--                LPVOID parameter // NULL pointer
--
-- RETURNS: DWORD // thread exit code
--
-- NOTES:
--    Initializes multicast variables and starts the playback thread and receive function
-------------------------------------------------------------------------------------------------*/
DWORD WINAPI JoinMulticast(LPVOID parameter)
{
	DWORD recvThread;
	BOOL flag = true;

	socketInfo.socket = socket(AF_INET, SOCK_DGRAM, 0);
	if (socketInfo.socket == INVALID_SOCKET)
	{
		cerr << "Multicast: socket creation failure (" << WSAGetLastError() << ")" << endl;
		return -1;
	}

	if (setsockopt(socketInfo.socket, SOL_SOCKET, SO_REUSEADDR, (char *)&flag, sizeof(flag)) == SOCKET_ERROR)
	{
		cerr << "Multicast: socket options set err (" << WSAGetLastError() << ")" << endl;
		return -1;
	}

	socketInfo.sockAddr.sin_family = AF_INET;
	socketInfo.sockAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	socketInfo.sockAddr.sin_port = htons(MULTICAST_PORT);
	if (bind(socketInfo.socket, (struct sockaddr*)&(socketInfo.sockAddr), sizeof(socketInfo.sockAddr)) == SOCKET_ERROR)
	{
		cerr << "Multicast: bind error (" << WSAGetLastError() << ")" << endl;
		return -1;
	}

	socketInfo.addr.imr_multiaddr.s_addr = inet_addr("234.5.6.7");
	socketInfo.addr.imr_interface.s_addr = INADDR_ANY;

	if (setsockopt(socketInfo.socket, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *)&(socketInfo.addr), sizeof(socketInfo.addr)) == SOCKET_ERROR)
	{
		cerr << "Multicast: add membership error (" << WSAGetLastError() << ")" << endl;
		return -1;
	}


	//begin streaming audio
	DWORD threadId;
	if ((multiThread = CreateThread(NULL, 0, StartMulticastPlayback, NULL, 0, &threadId)) == NULL)
	{
		cerr << "Multicast: thread creation error" << endl;
		return -1;
	}

	//begin reading data from the server
	RecvMulticastData();

	return 0;
}

/*-------------------------------------------------------------------------------------------------
-- FUNCTION: StartMulticastPlayback
--
-- DATE: April 1, 2015
--
-- DESIGNER: Michael Chimick, Rhea Lauzon
--
-- PROGRAMMER: Michael Chimick, Rhea Lauzon
--
-- INTERFACE: DWORD WINAPI StartMulticastPlayback(LPVOID parameter)
--                LPVOID parameter // NULL pointer
--
-- RETURNS: DWORD // thread exit code, -1 on failure; 0 on success
--
-- NOTES:
--    Starts the multicast playback
-------------------------------------------------------------------------------------------------*/
DWORD WINAPI StartMulticastPlayback(LPVOID parameter)
{
	//create the wave header
	WAVEFORMATEX wavFormat;

	//set up the format
	wavFormat.nSamplesPerSec = SAMPLES_PER_SECOND;
	wavFormat.wBitsPerSample = BITS_PER_SAMPLE;
	wavFormat.nChannels = CHANNELS;
	wavFormat.cbSize = 0;
	wavFormat.wFormatTag = WAVE_FORMAT_PCM;
	wavFormat.nBlockAlign = wavFormat.nChannels * (wavFormat.wBitsPerSample / 8);
	wavFormat.nAvgBytesPerSec = wavFormat.nSamplesPerSec * wavFormat.wBitsPerSample;

	//open the media device for streaming to
	if (waveOutOpen(&multicastOutput, WAVE_MAPPER, &wavFormat, (DWORD)MultiPlaybackCallback, NULL, CALLBACK_FUNCTION) != MMSYSERR_NOERROR)
	{
		cerr << "Multicast: failed to open output device" << endl;
		return -1;
	}

	// Prepare the wave headers
	for (int i = 0; i < NUM_OUTPUT_BUFFERS; i++)
	{
		//malloc and clear the memory
		audioBuffers[i] = new WAVEHDR();
		ZeroMemory(audioBuffers[i], sizeof(WAVEHDR));

		//update the their buffers to a position in the tripple buffer
		audioBuffers[i]->lpData = multiBuffer.buf;
		audioBuffers[i]->dwBufferLength = MUSIC_BUFFER_SIZE;

		// Create the header
		if (waveOutPrepareHeader(multicastOutput, audioBuffers[i], sizeof(WAVEHDR)) != MMSYSERR_NOERROR)
		{
			cerr << "Multicast: failed to create output header" << endl;
			return -1;
		}
	}

	cout << "Ready to play music" << endl;

	//wait until we have two messages worth of data; this avoids crackle
	while (multiBuffer.pos < PRE_BUFFER_SIZE)
	{
		//wait for the buffer to be ready
	}

	for (int i = 0; i < NUM_OUTPUT_BUFFERS; i++)
	{
		waveOutWrite(multicastOutput, audioBuffers[i], sizeof(WAVEHDR));
	}

	return 0;
}

/*-------------------------------------------------------------------------------------------------
-- FUNCTION: RecvMulticastData
--
-- DATE: April 1, 2015
--
-- DESIGNER: Michael Chimick, Rhea Lauzon
--
-- PROGRAMMER: Michael Chimick, Rhea Lauzon
--
-- INTERFACE: void RecvMulticastData()
--
-- RETURNS: void
--
-- NOTES:
--    Runs a loop that receives voice data and places it into the buffer
-------------------------------------------------------------------------------------------------*/
void RecvMulticastData()
{
	streaming = true;

	//receive data from the server
	int serverInfoSize = sizeof(socketInfo.sockAddr);
	int numReceived = 0;
	char tempBuffer[MESSAGE_SIZE];

	while (streaming)
	{
		if ((numReceived = recvfrom(socketInfo.socket, tempBuffer, MESSAGE_SIZE,\
			0, (struct sockaddr*) &socketInfo.sockAddr, &serverInfoSize)) < 0)
		{
			cerr << "Multicast: error reading data from multicast socket." << endl;
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

/*-------------------------------------------------------------------------------------------------
-- FUNCTION: MultiPlaybackCallback
--
-- DATE: April 1, 2015
--
-- DESIGNER: Michael Chimick
--
-- PROGRAMMER: Michael Chimick
--
-- INTERFACE: void CALLBACK MultiPlaybackCallback(HWAVEOUT hWave, UINT uMsg, DWORD dwUser, DWORD dw1, DWORD dw2)
--                HWAVEOUT hWave // handle to the waveform-audio device associated with the callback
--                UINT uMsg      // waveform-audio output message
--                DWORD dwUser   // user-instance data
--                DWORD dw1      // message parameter; wavehdr outputted
--                DWORD dw2      // message parameter
--
-- RETURNS: void
--
-- NOTES:
--    Callback after a buffer has finished playing, simply adds it to the back of the queue
-------------------------------------------------------------------------------------------------*/
void CALLBACK MultiPlaybackCallback(HWAVEOUT hWave, UINT uMsg, DWORD dwUser, DWORD dw1, DWORD dw2)
{
	if (uMsg == WOM_DONE)
	{
		if (waveOutWrite(hWave, (LPWAVEHDR)dw1, sizeof(WAVEHDR)) != MMSYSERR_NOERROR)
		{
			cerr << "Multicast: audio playback error" << endl;
			exit(1);
		}
	}
}


