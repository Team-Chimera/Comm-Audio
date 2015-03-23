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
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <winnetwk.h>
#include <ws2spi.h>
#include <wtsapi32.h>
#include "client.h"
#include "voiceChat.h"
#include "privateVoiceChat.h"

SOCKET_INFORMATION * recvSocketInfo;
TRIPLE_BUFFER * outBuffers;
HANDLE recvVoiceThread;
HANDLE recvParentThread;

SOCKET_INFORMATION * sendSocketInfo;
TRIPLE_BUFFER * inBuffers;
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
	closesocket(recvSocketInfo->socket);
	closesocket(recvSocketInfo->socket);

	if (TerminateThread(recvVoiceThread, 0) == 0)
	{
		cerr << "VoiceChat: terminate thread error (" << WSAGetLastError() << ")" << endl;
	}

	if (TerminateThread(sendVoiceThread, 0) == 0)
	{
		cerr << "VoiceChat: terminate thread error (" << WSAGetLastError() << ")" << endl;
	}

	if (TerminateThread(recvParentThread, 0) == 0)
	{
		cerr << "VoiceChat: terminate thread error (" << WSAGetLastError() << ")" << endl;
	}

	if (TerminateThread(sendParentThread, 0) == 0)
	{
		cerr << "VoiceChat: terminate thread error (" << WSAGetLastError() << ")" << endl;
	}

	waveOutUnprepareHeader(outBuffers->waveout, outBuffers->primary, sizeof(*(outBuffers->primary)));
	waveOutUnprepareHeader(outBuffers->waveout, outBuffers->secondary, sizeof(*(outBuffers->secondary)));
	waveOutUnprepareHeader(outBuffers->waveout, outBuffers->tertiary, sizeof(*(outBuffers->tertiary)));
	waveOutClose(outBuffers->waveout);

	waveInUnprepareHeader(inBuffers->wavein, inBuffers->primary, sizeof(*(inBuffers->primary)));
	waveInUnprepareHeader(inBuffers->wavein, inBuffers->secondary, sizeof(*(inBuffers->secondary)));
	waveInUnprepareHeader(inBuffers->wavein, inBuffers->tertiary, sizeof(*(inBuffers->tertiary)));
	waveInClose(inBuffers->wavein);

	delete outBuffers->buf;
	delete outBuffers->primary->lpData;
	delete outBuffers->secondary->lpData;
	delete outBuffers->tertiary->lpData;
	delete outBuffers;

	delete inBuffers->buf;
	delete inBuffers->primary->lpData;
	delete inBuffers->secondary->lpData;
	delete inBuffers->tertiary->lpData;
	delete inBuffers;

	delete recvSocketInfo->datagram.buf;
	delete recvSocketInfo;

	delete sendSocketInfo->datagram.buf;
	delete sendSocketInfo;

	outBuffers = NULL;
	inBuffers = NULL;
	recvSocketInfo = NULL;
	sendSocketInfo = NULL;

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

	if (recvSocketInfo != NULL) return -1;

	if (StartVoiceOut() == false) return -1;

	recvSocketInfo->socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
	if (recvSocketInfo->socket == INVALID_SOCKET)
	{
		cerr << "VoiceChat: Socket creation failure (" << WSAGetLastError() << ")" << endl;
		return -1;
	}

	if (setsockopt(recvSocketInfo->socket, SOL_SOCKET, SO_REUSEADDR, (char *)&flag, sizeof(flag)) == SOCKET_ERROR)
	{
		cerr << "VoiceChat: Socket options set err (" << WSAGetLastError() << ")" << endl;
		return -1;
	}

	recvSocketInfo->sockAddr.sin_family = AF_INET;
	recvSocketInfo->sockAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	recvSocketInfo->sockAddr.sin_port = htons(MULTICAST_PORT);
	if (bind(recvSocketInfo->socket, (struct sockaddr*)&(recvSocketInfo->sockAddr), sizeof(recvSocketInfo->sockAddr)) == SOCKET_ERROR)
	{
		cerr << "VoiceChat: Bind error (" << WSAGetLastError() << ")" << endl;
		return -1;
	}

	recvSocketInfo->addr.imr_multiaddr = *dest;
	recvSocketInfo->addr.imr_interface.S_un.S_addr = INADDR_ANY;

	if (setsockopt(recvSocketInfo->socket, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *)&(recvSocketInfo->addr), sizeof(recvSocketInfo->addr)) == SOCKET_ERROR)
	{
		cerr << "VoiceChat: add membership error (" << WSAGetLastError() << ")" << endl;
		return -1;
	}

	recvVoiceThread = CreateThread(NULL, 0, RecvVoiceThread, NULL, 0, &recvThread);
	if (recvVoiceThread == NULL)
	{
		cerr << "VoiceChat: Thread creation error (" << WSAGetLastError() << ")" << endl;
		return -1;
	}

	while (true)
	{
		WaitForSingleObjectEx(recvVoiceThread, INFINITE, FALSE);
		GetExitCodeThread(recvVoiceThread, &recvThread);
		if (recvThread != STILL_ACTIVE) break;
	}

	return 1;
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
bool StartVoiceOut()
{
	MMRESULT result;

	WAVEFORMATEX wfx;
	wfx.nChannels = CHANNELS;
	wfx.nSamplesPerSec = SAMPLES_PER_SECOND;
	wfx.wFormatTag = WAVE_FORMAT_PCM;
	wfx.wBitsPerSample = BITS_PER_SAMPLE;
	wfx.cbSize = 0;
	wfx.nBlockAlign = wfx.nChannels * wfx.wBitsPerSample / 8;
	wfx.nAvgBytesPerSec = wfx.nSamplesPerSec * wfx.wBitsPerSample / 8;

	result = waveOutOpen(&(outBuffers->waveout), WAVE_MAPPER, &wfx, (DWORD)VoiceOutCallback, NULL, CALLBACK_FUNCTION);
	if ((result != MMSYSERR_NOERROR) || (outBuffers->waveout == NULL))
	{
		cerr << "VoiceChat: Open waveout error (" << result << ")" << endl;
	}

	recvSocketInfo->datagram.buf = new char[DATAGRAM];
	recvSocketInfo->datagram.len = DATAGRAM;

	outBuffers->primary = new WAVEHDR;
	outBuffers->secondary = new WAVEHDR;
	outBuffers->tertiary = new WAVEHDR;

	outBuffers->buf = new char[BUFFER];

	outBuffers->primary->lpData = outBuffers->buf;
	outBuffers->primary->dwBufferLength = BUFFER;

	memcpy_s(outBuffers->secondary, sizeof(*(outBuffers->secondary)), \
		outBuffers->primary, sizeof(*(outBuffers->primary)));
	memcpy_s(outBuffers->tertiary, sizeof(*(outBuffers->tertiary)), \
		outBuffers->primary, sizeof(*(outBuffers->primary)));

	result = waveOutPrepareHeader(outBuffers->waveout, outBuffers->primary, sizeof(*(outBuffers->primary)));
	result = waveOutPrepareHeader(outBuffers->waveout, outBuffers->secondary, sizeof(*(outBuffers->secondary)));
	result = waveOutPrepareHeader(outBuffers->waveout, outBuffers->tertiary, sizeof(*(outBuffers->tertiary)));
	if (result != MMSYSERR_NOERROR)
	{
		cerr << "VoiceChat: Error preparing headers (" << result << ")" << endl;
		return false;
	}

	result = waveOutWrite(outBuffers->waveout, outBuffers->primary, sizeof(*(outBuffers->primary)));
	result = waveOutWrite(outBuffers->waveout, outBuffers->secondary, sizeof(*(outBuffers->secondary)));
	result = waveOutWrite(outBuffers->waveout, outBuffers->tertiary, sizeof(*(outBuffers->tertiary)));
	if (result != MMSYSERR_NOERROR)
	{
		cerr << "VoiceChat: Error preparing headers (" << result << ")" << endl;
		return false;
	}

	return true;
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
DWORD WINAPI RecvVoiceThread(LPVOID parameter)
{
	WSARecv(recvSocketInfo->socket, &(recvSocketInfo->datagram), 1, &(recvSocketInfo->bytesRECV), 0, &(recvSocketInfo->overlapped), RecvVoice);
	while (true)
	{
		SleepEx(INFINITE, TRUE);
	}

	return 0;
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: RecvVoice
--
-- DATE: March 12, 2015
--
-- REVISIONS: Created March 10, 2015
--
-- DESIGNER: Michael Chimick
--
-- PROGRAMMER: Michael Chimick
--
-- INTERFACE: void CALLBACK RecvVoice(DWORD error, DWORD bytesTransferred, LPWSAOVERLAPPED overlapped, DWORD flags)
--
-- RETURNS: void
--
-- NOTES:
--
--
----------------------------------------------------------------------------------------------------------------------*/
void CALLBACK RecvVoice(DWORD error, DWORD bytesTransferred, LPWSAOVERLAPPED overlapped, DWORD flags)
{
	unsigned int spaceLeft;

	spaceLeft = (BUFFER - outBuffers->pos);
	if (recvSocketInfo->bytesRECV > spaceLeft)
	{
		memcpy(outBuffers->buf + outBuffers->pos, recvSocketInfo->datagram.buf, spaceLeft);
		outBuffers->pos = 0;
		recvSocketInfo->bytesRECV -= spaceLeft;
		recvSocketInfo->datagram.buf += spaceLeft;
	}
	memcpy(outBuffers->buf + outBuffers->pos, recvSocketInfo->datagram.buf, recvSocketInfo->bytesRECV);
	outBuffers->pos += recvSocketInfo->bytesRECV;
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
	MMRESULT result;

	if (uMsg == WOM_DONE)
	{
		result = waveOutWrite(outBuffers->waveout, (LPWAVEHDR)dw1, sizeof(*((LPWAVEHDR)dw1)));
		if (result != MMSYSERR_NOERROR)
			cout << "VoiceChat: playing buffer error (" << result << ")" << endl;
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

	sendSocketInfo->socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
	if (sendSocketInfo->socket == INVALID_SOCKET)
	{
		cerr << "VoiceChat: Socket creation failure (" << WSAGetLastError() << ")" << endl;
		return false;
	}

	if (setsockopt(sendSocketInfo->socket, SOL_SOCKET, SO_REUSEADDR, (char *)&flag, sizeof(flag)) == SOCKET_ERROR)
	{
		cerr << "VoiceChat: Socket options set err (" << WSAGetLastError() << ")" << endl;
		return false;
	}

	sendSocketInfo->sockAddr.sin_family = AF_INET;
	sendSocketInfo->sockAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	sendSocketInfo->sockAddr.sin_port = htons(MULTICAST_PORT);
	if (bind(sendSocketInfo->socket, (struct sockaddr*)&(sendSocketInfo->sockAddr), sizeof(sendSocketInfo->sockAddr)) == SOCKET_ERROR)
	{
		cerr << "VoiceChat: Bind error (" << WSAGetLastError() << ")" << endl;
	}

	sendSocketInfo->addr.imr_multiaddr = *dest;
	sendSocketInfo->addr.imr_interface.S_un.S_addr = INADDR_ANY;

	if (setsockopt(sendSocketInfo->socket, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *)&(sendSocketInfo->addr), sizeof(sendSocketInfo->addr)) == SOCKET_ERROR)
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

	WAVEFORMATEX wfx;
	wfx.nChannels = CHANNELS;
	wfx.nSamplesPerSec = SAMPLES_PER_SECOND;
	wfx.wFormatTag = WAVE_FORMAT_PCM;
	wfx.wBitsPerSample = BITS_PER_SAMPLE;
	wfx.cbSize = 0;
	wfx.nBlockAlign = wfx.nChannels * wfx.wBitsPerSample / 8;
	wfx.nAvgBytesPerSec = wfx.nSamplesPerSec * wfx.wBitsPerSample / 8;

	result = waveOutOpen(&(outBuffers->waveout), WAVE_MAPPER, &wfx, (DWORD)VoiceOutCallback, NULL, CALLBACK_FUNCTION);
	if ((result != MMSYSERR_NOERROR) || (outBuffers->waveout == NULL))
	{
		cerr << "VoiceChat: Open waveout error (" << result << ")" << endl;
	}

	sendSocketInfo->datagram.buf = new char[DATAGRAM];
	sendSocketInfo->datagram.len = DATAGRAM;

	inBuffers->primary = new WAVEHDR;
	inBuffers->secondary = new WAVEHDR;
	inBuffers->tertiary = new WAVEHDR;

	inBuffers->buf = new char[BUFFER];

	inBuffers->primary->lpData = outBuffers->buf;
	inBuffers->primary->dwBufferLength = BUFFER;

	memcpy_s(inBuffers->secondary, sizeof(*(inBuffers->secondary)), \
		inBuffers->primary, sizeof(*(inBuffers->primary)));
	memcpy_s(inBuffers->tertiary, sizeof(*(inBuffers->tertiary)), \
		inBuffers->primary, sizeof(*(inBuffers->primary)));

	result = waveInPrepareHeader(inBuffers->wavein, inBuffers->primary, sizeof(*(inBuffers->primary)));
	result = waveInPrepareHeader(inBuffers->wavein, inBuffers->secondary, sizeof(*(inBuffers->secondary)));
	result = waveInPrepareHeader(inBuffers->wavein, inBuffers->tertiary, sizeof(*(inBuffers->tertiary)));
	if (result != MMSYSERR_NOERROR)
	{
		cerr << "VoiceChat: Error preparing headers (" << result << ")" << endl;
		return false;
	}

	result = waveInAddBuffer(inBuffers->wavein, inBuffers->primary, sizeof(*(inBuffers->primary)));
	result = waveInAddBuffer(inBuffers->wavein, inBuffers->secondary, sizeof(*(inBuffers->secondary)));
	result = waveInAddBuffer(inBuffers->wavein, inBuffers->tertiary, sizeof(*(inBuffers->tertiary)));
	if (result != MMSYSERR_NOERROR)
	{
		cerr << "VoiceChat: Error adding buffers (" << result << ")" << endl;
		return false;
	}

	result = waveInStart(inBuffers->wavein);
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
