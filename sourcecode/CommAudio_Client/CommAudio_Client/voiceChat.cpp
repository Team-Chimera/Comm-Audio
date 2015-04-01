/*------------------------------------------------------------------------------------------------------------------
-- SOURCE FILE: voicecast.cpp
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
//#include <winnetwk.h>
//#include <ws2spi.h>
//#include <wtsapi32.h>
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
HANDLE recvVoiceThread;
HANDLE recvParentThread;

SOCKET_INFORMATION sendSocketInfo;
LPWAVEHDR sendBuffers[NUM_OUTPUT_BUFFERS];
CircularBuffer sendBuffer;
HWAVEIN voiceInput;
HANDLE sendVoiceThread;
HANDLE sendParentThread;

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: JoinVoiceChat
--
-- DATE: March 12, 2015
--
-- REVISIONS: Created March 10, 2015
--
-- DESIGNER: Michael Chimick
--
-- PROGRAMMER: Michael Chimick
--
-- INTERFACE: void JoinVoiceChat(SOCKET voicecast, in_addr group)
--
-- RETURNS: void
--
-- NOTES:
-- Joins the voicecast session, and starts voicecast processing
--
----------------------------------------------------------------------------------------------------------------------*/
bool StartVoiceChat(in_addr dest) // valid ip for the server
{
	DWORD thread;

	recvParentThread = CreateThread(NULL, 0, StartRecvVoice, NULL, 0, &thread);
	if (recvParentThread == NULL)
	{
		cerr << "VoiceChat: Thread creation error (" << WSAGetLastError() << ")" << endl;
		return false;
	}

	sendParentThread = CreateThread(NULL, 0, StartSendVoice, NULL, 0, &thread);
	if (sendParentThread == NULL)
	{
		cerr << "VoiceChat: Thread creation error (" << WSAGetLastError() << ")" << endl;
		return false;
	}

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

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: DropVoiceChat
--
-- DATE: March 12, 2015
--
-- REVISIONS: Created March 10, 2015
--
-- DESIGNER: Michael Chimick
--
-- PROGRAMMER: Michael Chimick
--
-- INTERFACE: void DropVoiceChat()
--
-- RETURNS: void
--
-- NOTES:
-- Drops the voicecast session, and stops voicecast processing
--
----------------------------------------------------------------------------------------------------------------------*/
bool EndVoiceChat()
{
	closesocket(recvSocketInfo.socket);
	closesocket(sendSocketInfo.socket);

	if (TerminateThread(recvVoiceThread, 0) == 0)
	{
		cerr << "VoiceChat: terminate thread error (" << WSAGetLastError() << ")" << endl;
		exit(1);
	}

	if (TerminateThread(sendVoiceThread, 0) == 0)
	{
		cerr << "VoiceChat: terminate thread error (" << WSAGetLastError() << ")" << endl;
		exit(1);
	}

	if (TerminateThread(recvParentThread, 0) == 0)
	{
		cerr << "VoiceChat: terminate thread error (" << WSAGetLastError() << ")" << endl;
		exit(1);
	}

	if (TerminateThread(sendParentThread, 0) == 0)
	{
		cerr << "VoiceChat: terminate thread error (" << WSAGetLastError() << ")" << endl;
		exit(1);
	}

	for (int i = 0; i < NUM_OUTPUT_BUFFERS; i++)
	{
		if (waveOutUnprepareHeader(voiceOutput, recvBuffers[i], sizeof(WAVEHDR)) != MMSYSERR_ERROR)
		{
			cerr << "VoiceChat: out unprepareheader error" << endl;
			exit(1);
		}

		delete recvBuffers[i];
	}

	for (int i = 0; i < NUM_OUTPUT_BUFFERS; i++)
	{
		if (waveInUnprepareHeader(voiceInput, sendBuffers[i], sizeof(WAVEHDR)) != MMSYSERR_ERROR)
		{
			cerr << "VoiceChat: in unprepareheader error" << endl;
			exit(1);
		}

		delete recvBuffers[i];
	}

	waveOutClose(voiceOutput);
	waveInClose(voiceInput);

	return true;
}

// *******************************************************************************************************************************

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: JoinVoiceChat
--
-- DATE: March 12, 2015
--
-- REVISIONS: Created March 10, 2015
--
-- DESIGNER: Michael Chimick
--
-- PROGRAMMER: Michael Chimick
--
-- INTERFACE: void JoinVoiceChat(SOCKET voicecast, in_addr group)
--
-- RETURNS: void
--
-- NOTES:
-- Joins the voicecast session, and starts voicecast processing
--
----------------------------------------------------------------------------------------------------------------------*/
DWORD WINAPI StartRecvVoice(LPVOID parameter)
{
	DWORD recvThread;
	BOOL flag = true;
	in_addr * dest = (in_addr*)parameter;

	if (StartVoiceOut() == false) return -1;

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
	recvSocketInfo.sockAddr.sin_port = htons(MULTICAST_PORT);
	if (bind(recvSocketInfo.socket, (struct sockaddr*)&(recvSocketInfo.sockAddr), sizeof(recvSocketInfo.sockAddr)) == SOCKET_ERROR)
	{
		cerr << "VoiceChat: Bind error (" << WSAGetLastError() << ")" << endl;
		return -1;
	}

	recvSocketInfo.addr.imr_multiaddr = *dest;
	recvSocketInfo.addr.imr_interface.s_addr = INADDR_ANY;

	if (setsockopt(recvSocketInfo.socket, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *)&(recvSocketInfo.addr), sizeof(recvSocketInfo.addr)) == SOCKET_ERROR)
	{
		cerr << "VoiceChat: add membership error (" << WSAGetLastError() << ")" << endl;
		return -1;
	}

	RecvVoiceData();

	return 1;
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: RecvVoiceThread
--
-- DATE: March 12, 2015
--
-- REVISIONS: Created March 10, 2015
--
-- DESIGNER: Michael Chimick
--
-- PROGRAMMER: Michael Chimick
--
-- INTERFACE: DWORD WINAPI RecvVoiceThread(LPVOID parameter)
--
-- RETURNS: DWORD
--
-- NOTES:
--
--
----------------------------------------------------------------------------------------------------------------------*/
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

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: JoinVoiceChat
--
-- DATE: March 12, 2015
--
-- REVISIONS: Created March 10, 2015
--
-- DESIGNER: Michael Chimick
--
-- PROGRAMMER: Michael Chimick
--
-- INTERFACE: void JoinVoiceChat(SOCKET voicecast, in_addr group)
--
-- RETURNS: void
--
-- NOTES:
-- Joins the voicecast session, and starts voicecast processing
--
----------------------------------------------------------------------------------------------------------------------*/
DWORD WINAPI StartVoiceOut(LPVOID parameter)
{
	WAVEFORMATEX wavFormat;
	wavFormat.nSamplesPerSec = SAMPLES_PER_SECOND;
	wavFormat.wBitsPerSample = BITS_PER_SAMPLE;
	wavFormat.nChannels = CHANNELS;
	wavFormat.cbSize = 0;
	wavFormat.wFormatTag = WAVE_FORMAT_PCM;
	wavFormat.nBlockAlign = wavFormat.nChannels * (wavFormat.wBitsPerSample / 8);
	wavFormat.nAvgBytesPerSec = wavFormat.nSamplesPerSec * wavFormat.wBitsPerSample;

	if (waveOutOpen(&voiceOutput, WAVE_MAPPER, &wavFormat, (DWORD)VoiceOutCallback, NULL, CALLBACK_FUNCTION) != MMSYSERR_NOERROR)
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

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION:
--
-- DATE: March 12, 2015
--
-- REVISIONS: Created March 10, 2015
--
-- DESIGNER: Michael Chimick
--
-- PROGRAMMER: Michael Chimick
--
-- INTERFACE:
--
-- RETURNS:
--
-- NOTES:
--
--
----------------------------------------------------------------------------------------------------------------------*/
void CALLBACK VoiceOutCallback(HWAVEOUT hWave, UINT uMsg, DWORD dwUser, DWORD dw1, DWORD dw2)
{
	if (uMsg == WOM_DONE)
	{
		if (waveOutWrite(voiceOutput, (LPWAVEHDR)dw1, sizeof(WAVEHDR)) != MMSYSERR_NOERROR)
		{
			cout << "VoiceChat: playing buffer error" << endl;
			exit(1);
		}
	}
}

// ***********************************************************************************************************************************************

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: JoinVoiceChat
--
-- DATE: March 12, 2015
--
-- REVISIONS: Created March 10, 2015
--
-- DESIGNER: Michael Chimick
--
-- PROGRAMMER: Michael Chimick
--
-- INTERFACE: void JoinVoiceChat(SOCKET voicecast, in_addr group)
--
-- RETURNS: void
--
-- NOTES:
-- Joins the voicecast session, and starts voicecast processing
--
----------------------------------------------------------------------------------------------------------------------*/
DWORD WINAPI StartSendVoice(LPVOID parameter)
{
	DWORD sendThread;

	BOOL flag = true;

	in_addr * dest = (in_addr*)parameter;

	if (sendSocketInfo != NULL) return false;

	if (StartVoiceIn() == false) return false;

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
	sendSocketInfo.sockAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	sendSocketInfo.sockAddr.sin_port = htons(MULTICAST_PORT);
	if (bind(sendSocketInfo.socket, (struct sockaddr*)&(sendSocketInfo.sockAddr), sizeof(sendSocketInfo.sockAddr)) == SOCKET_ERROR)
	{
		cerr << "VoiceChat: Bind error (" << WSAGetLastError() << ")" << endl;
	}

	sendSocketInfo.addr.imr_multiaddr = *dest;
	sendSocketInfo.addr.imr_interface.S_un.S_addr = INADDR_ANY;

	if (setsockopt(sendSocketInfo.socket, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *)&(sendSocketInfo.addr), sizeof(sendSocketInfo.addr)) == SOCKET_ERROR)
	{
		cerr << "VoiceChat: add membership error (" << WSAGetLastError() << ")" << endl;
	}

	/*sendVoiceThread = CreateThread(NULL, 0, SendVoiceThread, NULL, 0, &sendThread);
	if (sendVoiceThread == NULL)
	{
		cerr << "VoiceChat: Thread creation error (" << WSAGetLastError() << ")" << endl;
		return false;
	}

	while (true)
	{
		WaitForSingleObjectEx(sendVoiceThread, INFINITE, FALSE);
		GetExitCodeThread(sendVoiceThread, &sendThread);
		if (sendThread != STILL_ACTIVE) break;
	}*/

	return true;
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: JoinVoiceChat
--
-- DATE: March 12, 2015
--
-- REVISIONS: Created March 10, 2015
--
-- DESIGNER: Michael Chimick
--
-- PROGRAMMER: Michael Chimick
--
-- INTERFACE: void JoinVoiceChat(SOCKET voicecast, in_addr group)
--
-- RETURNS: void
--
-- NOTES:
-- Joins the voicecast session, and starts voicecast processing
--
----------------------------------------------------------------------------------------------------------------------*/
bool StartVoiceIn()
{
	MMRESULT result;

	WAVEFORMATEX wavFormat;
	wavFormat.nChannels = CHANNELS;
	wavFormat.nSamplesPerSec = SAMPLES_PER_SECOND;
	wavFormat.wFormatTag = WAVE_FORMAT_PCM;
	wavFormat.wBitsPerSample = BITS_PER_SAMPLE;
	wavFormat.cbSize = 0;
	wavFormat.nBlockAlign = wavFormat.nChannels * wavFormat.wBitsPerSample / 8;
	wavFormat.nAvgBytesPerSec = wavFormat.nSamplesPerSec * wavFormat.wBitsPerSample / 8;

	result = waveOutOpen(&(outBuffers.waveout), WAVE_MAPPER, &wavFormat, (DWORD)VoiceOutCallback, NULL, CALLBACK_FUNCTION);
	if ((result != MMSYSERR_NOERROR) || (outBuffers.waveout == NULL))
	{
		cerr << "VoiceChat: Open waveout error (" << result << ")" << endl;
	}

	sendSocketInfo.datagram.buf = new char[DATAGRAM];
	sendSocketInfo.datagram.len = DATAGRAM;

	inBuffers.primary = new WAVEHDR;
	inBuffers.secondary = new WAVEHDR;
	inBuffers.tertiary = new WAVEHDR;

	inBuffers.buf = new char[BUFFER];

	inBuffers.primary.lpData = outBuffers.buf;
	inBuffers.primary.dwBufferLength = BUFFER;

	memcpy_s(inBuffers.secondary, sizeof(*(inBuffers.secondary)), \
		inBuffers.primary, sizeof(*(inBuffers.primary)));
	memcpy_s(inBuffers.tertiary, sizeof(*(inBuffers.tertiary)), \
		inBuffers.primary, sizeof(*(inBuffers.primary)));

	result = waveInPrepareHeader(inBuffers.wavein, inBuffers.primary, sizeof(*(inBuffers.primary)));
	result = waveInPrepareHeader(inBuffers.wavein, inBuffers.secondary, sizeof(*(inBuffers.secondary)));
	result = waveInPrepareHeader(inBuffers.wavein, inBuffers.tertiary, sizeof(*(inBuffers.tertiary)));
	if (result != MMSYSERR_NOERROR)
	{
		cerr << "VoiceChat: Error preparing headers (" << result << ")" << endl;
		return false;
	}

	result = waveInAddBuffer(inBuffers.wavein, inBuffers.primary, sizeof(*(inBuffers.primary)));
	result = waveInAddBuffer(inBuffers.wavein, inBuffers.secondary, sizeof(*(inBuffers.secondary)));
	result = waveInAddBuffer(inBuffers.wavein, inBuffers.tertiary, sizeof(*(inBuffers.tertiary)));
	if (result != MMSYSERR_NOERROR)
	{
		cerr << "VoiceChat: Error adding buffers (" << result << ")" << endl;
		return false;
	}

	result = waveInStart(inBuffers.wavein);
	if (result != MMSYSERR_NOERROR)
	{
		cerr << "VoiceChat: Error starting recording (" << result << ")" << endl;
		return false;
	}

	return true;
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION:
--
-- DATE: March 12, 2015
--
-- REVISIONS: Created March 10, 2015
--
-- DESIGNER: Michael Chimick
--
-- PROGRAMMER: Michael Chimick
--
-- INTERFACE:
--
-- RETURNS:
--
-- NOTES:
--
--
----------------------------------------------------------------------------------------------------------------------*/
void CALLBACK VoiceInCallback(HWAVEOUT hWave, UINT uMsg, DWORD dwUser, DWORD dw1, DWORD dw2)
{
	//
}
