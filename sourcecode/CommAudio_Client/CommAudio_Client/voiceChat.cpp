/*-------------------------------------------------------------------------------------------------
-- SOURCE FILE: voiceChat.cpp
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
#include "voiceChat.h"

using std::cout;
using std::cerr;
using std::endl;

SOCKET_INFORMATION recvSocketInfo;
LPWAVEHDR recvBuffers[NUM_OUTPUT_BUFFERS];
CircularBuffer recvCBuffer;
HWAVEOUT voiceOutput;
HANDLE recvThread;
HANDLE recvParentThread;

SOCKET_INFORMATION sendSocketInfo;
LPWAVEHDR sendBuffers[NUM_OUTPUT_BUFFERS];
CircularBuffer sendCBuffer;
HWAVEIN voiceInput;
HANDLE sendThread;
HANDLE sendParentThread;

/******************************************************************************************************************************************
*******************************************************************************************************************************************
 *
 * START AND END FUNCTIONS
 *
*******************************************************************************************************************************************
******************************************************************************************************************************************/

/*-------------------------------------------------------------------------------------------------
-- FUNCTION: StartVoiceChat
--
-- DATE: April 1, 2015
--
-- DESIGNER: Michael Chimick
--
-- PROGRAMMER: Michael Chimick
--
-- INTERFACE: bool StartVoiceChat(hostent * dest)
--                hostent * dest // valid ip for the server
--
-- RETURNS: bool // returns true is all calls successful, false otherwise
--
-- NOTES:
--    Starts receiving, playback, recording, and sending of voice chat data
-------------------------------------------------------------------------------------------------*/
bool StartVoiceChat(hostent * dest) // valid ip for the server
{
	DWORD thread;

	recvParentThread = CreateThread(NULL, 0, StartVoiceRecv, NULL, 0, &thread);
	if (recvParentThread == NULL)
	{
		cerr << "VoiceChat: Thread creation error (" << WSAGetLastError() << ")" << endl;
		return false;
	}

	sendParentThread = CreateThread(NULL, 0, StartVoiceSend, dest, 0, &thread);
	if (sendParentThread == NULL)
	{
		cerr << "VoiceChat: Thread creation error (" << WSAGetLastError() << ")" << endl;
		return false;
	}

	// these next two aren't strictly nessesary, but they protect against a specific occurrance
	GetExitCodeThread(recvParentThread, &thread);
	if (thread != STILL_ACTIVE)
	{
		cerr << "VoiceChat: Parent start error (" << WSAGetLastError() << ")" << endl;
		return false;
	}

	GetExitCodeThread(sendParentThread, &thread);
	if (thread != STILL_ACTIVE)
	{
		cerr << "VoiceChat: Parent start error (" << WSAGetLastError() << ")" << endl;
		return false;
	}

	return true;
}

/*-------------------------------------------------------------------------------------------------
-- FUNCTION: EndVoiceChat
--
-- DATE: April 1, 2015
--
-- DESIGNER: Michael Chimick
--
-- PROGRAMMER: Michael Chimick
--
-- INTERFACE: bool EndVoiceChat()
--
-- RETURNS: bool // returns true is all calls successful, false otherwise
--
-- NOTES:
--    Terminates the voice chat threads, and resets necessary variables
-------------------------------------------------------------------------------------------------*/
bool EndVoiceChat()
{
	closesocket(recvSocketInfo.socket);
	closesocket(sendSocketInfo.socket);

	if (TerminateThread(recvThread, 0) == 0)
	{
		cerr << "VoiceChat: terminate thread error (" << WSAGetLastError() << ")" << endl;
		return false;
	}

	if (TerminateThread(sendThread, 0) == 0)
	{
		cerr << "VoiceChat: terminate thread error (" << WSAGetLastError() << ")" << endl;
		return false;
	}

	if (TerminateThread(recvParentThread, 0) == 0)
	{
		cerr << "VoiceChat: terminate thread error (" << WSAGetLastError() << ")" << endl;
		return false;
	}

	if (TerminateThread(sendParentThread, 0) == 0)
	{
		cerr << "VoiceChat: terminate thread error (" << WSAGetLastError() << ")" << endl;
		return false;
	}

	for (int i = 0; i < NUM_OUTPUT_BUFFERS; i++)
	{
		if (waveOutUnprepareHeader(voiceOutput, recvBuffers[i], sizeof(WAVEHDR)) != MMSYSERR_ERROR)
		{
			cerr << "VoiceChat: out unprepareheader error" << endl;
			return false;
		}

		delete recvBuffers[i];
	}

	for (int i = 0; i < NUM_OUTPUT_BUFFERS; i++)
	{
		if (waveInUnprepareHeader(voiceInput, sendBuffers[i], sizeof(WAVEHDR)) != MMSYSERR_ERROR)
		{
			cerr << "VoiceChat: in unprepareheader error" << endl;
			return false;
		}

		delete recvBuffers[i];
	}

	waveOutClose(voiceOutput);
	waveInClose(voiceInput);

	return true;
}

/******************************************************************************************************************************************
*******************************************************************************************************************************************
*
* RECEIVE AND PLAY FUNCTIONS
*
*******************************************************************************************************************************************
******************************************************************************************************************************************/

/*-------------------------------------------------------------------------------------------------
-- FUNCTION: StartVoiceRecv
--
-- DATE: April 1, 2015
--
-- DESIGNER: Michael Chimick
--
-- PROGRAMMER: Michael Chimick
--
-- INTERFACE: DWORD WINAPI StartVoiceRecv(LPVOID parameter)
--                LPVOID parameter // NULL pointer
--
-- RETURNS: DWORD // thread exit code
--
-- NOTES:
--    Initializes variables and starts the receive and playback functions
-------------------------------------------------------------------------------------------------*/
DWORD WINAPI StartVoiceRecv(LPVOID parameter)
{
	BOOL flag = true;

	recvSocketInfo.socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
	if (recvSocketInfo.socket == INVALID_SOCKET)
	{
		cerr << "VoiceChat: Socket creation failure (" << WSAGetLastError() << ")" << endl;
		return -1;
	}

	if (setsockopt(recvSocketInfo.socket, SOL_SOCKET, SO_REUSEADDR, (char *)&flag, sizeof(flag)) == SOCKET_ERROR)
	{
		cerr << "VoiceChat: Socket options set err (" << WSAGetLastError() << ")" << endl;
		return -1;
	}

	recvSocketInfo.sockAddr.sin_family = AF_INET;
	recvSocketInfo.sockAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	recvSocketInfo.sockAddr.sin_port = htons(VOICECHAT_PORT);
	if (bind(recvSocketInfo.socket, (struct sockaddr*)&(recvSocketInfo.sockAddr), sizeof(recvSocketInfo.sockAddr)) == SOCKET_ERROR)
	{
		cerr << "VoiceChat: Bind error (" << WSAGetLastError() << ")" << endl;
		return -1;
	}

	DWORD threadId;
	if ((recvThread = CreateThread(NULL, 0, StartVoicePlayback, NULL, 0, &threadId)) == NULL)
	{
		cerr << "VoiceChat: thread creation error" << endl;
		return -1;
	}

	RecvVoiceData();

	return 1;
}

/*-------------------------------------------------------------------------------------------------
-- FUNCTION: StartVoicePlayback
--
-- DATE: April 1, 2015
--
-- DESIGNER: Michael Chimick
--
-- PROGRAMMER: Michael Chimick
--
-- INTERFACE: DWORD WINAPI StartVoicePlayback(LPVOID parameter)
--                LPVOID parameter // NULL pointer
--
-- RETURNS: DWORD // thread exit code
--
-- NOTES:
--    Starts the voice playback
-------------------------------------------------------------------------------------------------*/
DWORD WINAPI StartVoicePlayback(LPVOID parameter)
{
	WAVEFORMATEX wavFormat;
	wavFormat.nSamplesPerSec = SAMPLES_PER_SECOND;
	wavFormat.wBitsPerSample = BITS_PER_SAMPLE;
	wavFormat.nChannels = CHANNELS;
	wavFormat.cbSize = 0;
	wavFormat.wFormatTag = WAVE_FORMAT_PCM;
	wavFormat.nBlockAlign = wavFormat.nChannels * (wavFormat.wBitsPerSample / 8);
	wavFormat.nAvgBytesPerSec = wavFormat.nSamplesPerSec * wavFormat.wBitsPerSample;

	if (waveOutOpen(&voiceOutput, WAVE_MAPPER, &wavFormat, (DWORD)VoicePlaybackCallback, NULL, CALLBACK_FUNCTION) != MMSYSERR_NOERROR)
	{
		cerr << "VoiceChat: open waveout error" << endl;
		return -1;
	}

	for (int i = 0; i < NUM_OUTPUT_BUFFERS; i++)
	{
		recvBuffers[i] = new WAVEHDR();
		ZeroMemory(recvBuffers[i], sizeof(WAVEHDR));

		recvBuffers[i]->lpData = recvCBuffer.buf;
		recvBuffers[i]->dwBufferLength = MUSIC_BUFFER_SIZE;

		if (waveOutPrepareHeader(voiceOutput, recvBuffers[i], sizeof(WAVEHDR)) != MMSYSERR_ERROR)
		{
			cerr << "VoiceChat: error preparing out headers" << endl;
			return -1;
		}
	}

	while (recvCBuffer.pos < PRE_BUFFER_SIZE) { /* wait */ }

	for (int i = 0; i < NUM_OUTPUT_BUFFERS; i++)
	{
		if (waveOutWrite(voiceOutput, recvBuffers[i], sizeof(WAVEHDR)) != MMSYSERR_ERROR)
		{
			cerr << "VoiceChat: error out write" << endl;
			return -1;
		}
	}

	return 0;
}

/*-------------------------------------------------------------------------------------------------
-- FUNCTION: RecvVoiceData
--
-- DATE: April 1, 2015
--
-- DESIGNER: Michael Chimick
--
-- PROGRAMMER: Michael Chimick
--
-- INTERFACE: void RecvVoiceData()
--
-- RETURNS: void
--
-- NOTES:
--    Runs a loop that receives voice data and places it into the buffer
-------------------------------------------------------------------------------------------------*/
void RecvVoiceData()
{
	int serverInfoSize = sizeof(recvSocketInfo.sockAddr);
	int numReceived = 0;
	char tempBuffer[MESSAGE_SIZE];

	while (true)
	{
		if ((numReceived = recvfrom(recvSocketInfo.socket, tempBuffer, MESSAGE_SIZE, \
			0, (struct sockaddr*)&recvSocketInfo.sockAddr, &serverInfoSize)) < 0)
		{
			cerr << "VoiceChat: recvfrom error" << endl;
			continue;
		}

		for (int i = 0; i < numReceived; i++)
		{
			recvCBuffer.buf[recvCBuffer.pos] = tempBuffer[i];
			if (recvCBuffer.pos == MUSIC_BUFFER_SIZE - 1)
			{
				recvCBuffer.pos = 0;
			}
			else
			{
				recvCBuffer.pos++;
			}
		}
	}
}

/*-------------------------------------------------------------------------------------------------
-- FUNCTION: VoicePlaybackCallback
--
-- DATE: April 1, 2015
--
-- DESIGNER: Michael Chimick
--
-- PROGRAMMER: Michael Chimick
--
-- INTERFACE: void CALLBACK VoicePlaybackCallback(HWAVEOUT hWave, UINT uMsg, DWORD dwUser, DWORD dw1, DWORD dw2)
--                HWAVEOUT hWave // handle to the waveform-audio device associated with the callback
--                UINT uMsg      // waveform-audio output message
--                DWORD dwUser   // user-instance data
--                DWORD dw1      // message parameter
--                DWORD dw2      // message parameter
--
-- RETURNS: void
--
-- NOTES:
--    Callback after a buffer has finished playing, simply adds it to the back of the queue
-------------------------------------------------------------------------------------------------*/
void CALLBACK VoicePlaybackCallback(HWAVEOUT hWave, UINT uMsg, DWORD dwUser, DWORD dw1, DWORD dw2)
{
	if (uMsg == WOM_DONE)
	{
		if (waveOutWrite(hWave, (LPWAVEHDR)dw1, sizeof(WAVEHDR)) != MMSYSERR_NOERROR)
		{
			cout << "VoiceChat: playing buffer error" << endl;
			exit(1);
		}
	}
}

/******************************************************************************************************************************************
*******************************************************************************************************************************************
*
* RECORD AND SEND FUNCTIONS
*
*******************************************************************************************************************************************
******************************************************************************************************************************************/

/*-------------------------------------------------------------------------------------------------
-- FUNCTION: StartVoiceSend
--
-- DATE: April 1, 2015
--
-- DESIGNER: Michael Chimick
--
-- PROGRAMMER: Michael Chimick
--
-- INTERFACE: DWORD WINAPI StartVoiceSend(LPVOID parameter)
--                LPVOID parameter // NULL pointer
--
-- RETURNS: void
--
-- NOTES:
--    Initializes variables and starts the record and send functions
-------------------------------------------------------------------------------------------------*/
DWORD WINAPI StartVoiceSend(LPVOID parameter)
{
	BOOL flag = true;
	hostent * dest = (hostent*)parameter;

	sendSocketInfo.socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
	if (sendSocketInfo.socket == INVALID_SOCKET)
	{
		cerr << "VoiceChat: Socket creation failure (" << WSAGetLastError() << ")" << endl;
		return false;
	}

	if (setsockopt(sendSocketInfo.socket, SOL_SOCKET, SO_REUSEADDR, (char *)&flag, sizeof(flag)) == SOCKET_ERROR)
	{
		cerr << "VoiceChat: Socket options set err (" << WSAGetLastError() << ")" << endl;
		return false;
	}

	sendSocketInfo.sockAddr.sin_family = AF_INET;
	sendSocketInfo.sockAddr.sin_port = htons(VOICECHAT_PORT);
	memcpy((char *)&sendSocketInfo.sockAddr.sin_addr, dest->h_addr, dest->h_length);

	if (bind(sendSocketInfo.socket, (struct sockaddr*)&(sendSocketInfo.sockAddr), sizeof(sendSocketInfo.sockAddr)) == SOCKET_ERROR)
	{
		cerr << "VoiceChat: Bind error (" << WSAGetLastError() << ")" << endl;
	}

	DWORD threadID;
	if ((sendThread = CreateThread(NULL, 0, StartVoiceRecord, NULL, 0, &threadID)) == NULL)
	{
		cerr << "VoiceChat: thread creation error" << endl;
	}

	return true;
}

/*-------------------------------------------------------------------------------------------------
-- FUNCTION: StartVoiceRecord
--
-- DATE: April 1, 2015
--
-- DESIGNER: Michael Chimick
--
-- PROGRAMMER: Michael Chimick
--
-- INTERFACE: DWORD WINAPI StartVoiceRecord(LPVOID parameter)
--                LPVOID parameter // NULL pointer
--
-- RETURNS: void
--
-- NOTES:
--    Starts the voice recording
-------------------------------------------------------------------------------------------------*/
DWORD WINAPI StartVoiceRecord(LPVOID parameter)
{
	WAVEFORMATEX wavFormat;
	wavFormat.nSamplesPerSec = SAMPLES_PER_SECOND;
	wavFormat.wBitsPerSample = BITS_PER_SAMPLE;
	wavFormat.nChannels = CHANNELS;
	wavFormat.cbSize = 0;
	wavFormat.wFormatTag = WAVE_FORMAT_PCM;
	wavFormat.nBlockAlign = wavFormat.nChannels * (wavFormat.wBitsPerSample / 8);
	wavFormat.nAvgBytesPerSec = wavFormat.nSamplesPerSec * wavFormat.wBitsPerSample;

	if (waveInOpen(&voiceInput, WAVE_MAPPER, &wavFormat, (DWORD)VoiceRecordCallback, NULL, CALLBACK_FUNCTION) != MMSYSERR_NOERROR)
	{
		cerr << "VoiceChat: Open wavein error" << endl;
		return -1;
	}

	for (int i = 0; i < NUM_OUTPUT_BUFFERS; i++)
	{
		sendBuffers[i] = new WAVEHDR();
		ZeroMemory(sendBuffers[i], sizeof(WAVEHDR));

		sendBuffers[i]->lpData = sendCBuffer.buf;
		sendBuffers[i]->dwBufferLength = MUSIC_BUFFER_SIZE;

		if (waveInPrepareHeader(voiceInput, sendBuffers[i], sizeof(WAVEHDR)) != MMSYSERR_ERROR)
		{
			cerr << "VoiceChat: error preparing in headers" << endl;
			return -1;
		}

		if (waveInAddBuffer(voiceInput, sendBuffers[i], sizeof(WAVEHDR)) != MMSYSERR_ERROR)
		{
			cerr << "VoiceChat: error adding in buffers to queue" << endl;
			return -1;
		}
	}

	if (waveInStart(voiceInput) != MMSYSERR_NOERROR)
	{
		cerr << "VoiceChat: Error starting recording" << endl;
		return false;
	}

	return true;
}

/*-------------------------------------------------------------------------------------------------
-- FUNCTION: VoiceRecordCallback
--
-- DATE: April 1, 2015
--
-- REVISIONS: Created March 10, 2015
--
-- DESIGNER: Michael Chimick
--
-- PROGRAMMER: Michael Chimick
--
-- INTERFACE: void CALLBACK VoiceRecordCallback(HWAVEOUT hWave, UINT uMsg, DWORD dwUser, DWORD dw1, DWORD dw2)
--                HWAVEOUT hWave // handle to the waveform-audio device associated with the callback
--                UINT uMsg      // waveform-audio output message
--                DWORD dwUser   // user-instance data
--                DWORD dw1      // message parameter
--                DWORD dw2      // message parameter
--
-- RETURNS:
--
-- NOTES:
--    Callback after a buffer has been filled, 
--    sends it to the server and then adds it back to the queue
-------------------------------------------------------------------------------------------------*/
void CALLBACK VoiceRecordCallback(HWAVEOUT hWave, UINT uMsg, DWORD dwUser, DWORD dw1, DWORD dw2)
{
	if (uMsg == WIM_DATA)
	{
		// send the data to the server
		if (sendto(sendSocketInfo.socket, ((LPWAVEHDR)dw1)->lpData, ((LPWAVEHDR)dw1)->dwBufferLength, 0, \
			(struct sockaddr *)&sendSocketInfo.sockAddr, sizeof(sendSocketInfo.sockAddr)) == SOCKET_ERROR)
		{
			cerr << "VoiceChat: send error (" << WSAGetLastError() << ")" << endl;
			exit(1);
		}

		// add the buffer back onto the queue
		if (waveInAddBuffer(hWave, (LPWAVEHDR)dw1, sizeof(WAVEHDR)) != MMSYSERR_NOERROR)
		{
			cout << "VoiceChat: error adding in buffer to queue" << endl;
			exit(1);
		}
	}
}
