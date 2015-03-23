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

#include "multicast.h"
#include <iostream>

using std::cout;
using std::cerr;
using std::endl;

SOCKET_INFORMATION * socketInfo;
TRIPLE_BUFFER * buffers;
HANDLE multiThread;
HANDLE multiParentThread;

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

    multiParentThread = CreateThread(NULL, 0, JoinMulticast, (void *) &group, 0, &thread);
	if (multiParentThread == NULL)
	{
		cerr << "VoiceChat: Thread creation error (" << WSAGetLastError() << ")" << endl;
		return false;
	}

	GetExitCodeThread(multiParentThread, &thread);
	if (thread != STILL_ACTIVE)
	{
		cerr << "VoiceChat: Parent start error (" << WSAGetLastError() << ")" << endl;
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
	if (setsockopt(socketInfo->socket, IPPROTO_IP, IP_DROP_MEMBERSHIP, (char *)&(socketInfo->addr), sizeof(socketInfo->addr)) == SOCKET_ERROR)
	{
		cerr << "Multicast: drop membership error (" << WSAGetLastError() << ")" << endl;
		return false;
	}

	closesocket(socketInfo->socket);

	if (TerminateThread(multiThread, 0) == 0)
	{
		cerr << "Multicast: terminate thread error (" << WSAGetLastError() << ")" << endl;
	}

	if (TerminateThread(multiParentThread, 0) == 0)
	{
		cerr << "Multicast: terminate thread error (" << WSAGetLastError() << ")" << endl;
	}

	waveOutUnprepareHeader(buffers->waveout, buffers->primary, sizeof(*(buffers->primary)));
	waveOutUnprepareHeader(buffers->waveout, buffers->secondary, sizeof(*(buffers->secondary)));
	waveOutUnprepareHeader(buffers->waveout, buffers->tertiary, sizeof(*(buffers->tertiary)));
	waveOutClose(buffers->waveout);

	delete buffers->buf;
	delete buffers->primary->lpData;
	delete buffers->secondary->lpData;
	delete buffers->tertiary->lpData;
	delete buffers;

	delete socketInfo->datagram.buf;
	delete socketInfo;

	buffers = NULL;
	socketInfo = NULL;

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

	if (socketInfo != NULL) return -1;

	if (StartWaveOut() == false) return -1;

	socketInfo->socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
	if (socketInfo->socket == INVALID_SOCKET)
	{
		cerr << "Multicast: Socket creation failure (" << WSAGetLastError() << ")" << endl;
		return -1;
	}

	if (setsockopt(socketInfo->socket, SOL_SOCKET, SO_REUSEADDR, (char *)&flag, sizeof(flag)) == SOCKET_ERROR)
	{
		cerr << "Multicast: Socket options set err (" << WSAGetLastError() << ")" << endl;
		return -1;
	}

	socketInfo->sockAddr.sin_family = AF_INET;
	socketInfo->sockAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	socketInfo->sockAddr.sin_port = htons(MULTICAST_PORT);
	if (bind(socketInfo->socket, (struct sockaddr*)&(socketInfo->sockAddr), sizeof(socketInfo->sockAddr)) == SOCKET_ERROR)
	{
		cerr << "Multicast: Bind error (" << WSAGetLastError() << ")" << endl;
		return -1;
	}

	socketInfo->addr.imr_multiaddr = *group;
	socketInfo->addr.imr_interface.S_un.S_addr = INADDR_ANY;

	if (setsockopt(socketInfo->socket, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *)&(socketInfo->addr), sizeof(socketInfo->addr)) == SOCKET_ERROR)
	{
		cerr << "Multicast: add membership error (" << WSAGetLastError() << ")" << endl;
		return -1;
	}

	multiThread = CreateThread(NULL, 0, RecvMultiThread, NULL, 0, &recvThread);
	if (multiThread == NULL)
	{
		cerr << "Multicast: Thread creation error (" << WSAGetLastError() << ")" << endl;
		return -1;
	}

	while (true)
	{
		WaitForSingleObjectEx(multiThread, INFINITE, FALSE);
		GetExitCodeThread(multiThread, &recvThread);
		if (recvThread != STILL_ACTIVE) break;
	}

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
bool StartWaveOut()
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

	result = waveOutOpen(&(buffers->waveout), WAVE_MAPPER, &wfx, (DWORD)MultiWaveCallback, NULL, CALLBACK_FUNCTION);
	if ((result != MMSYSERR_NOERROR) || (buffers->waveout == NULL))
	{
		cerr << "Multicast: Open waveout error (" << result << ")" << endl;
	}

	socketInfo->datagram.buf = new char[DATAGRAM];
	socketInfo->datagram.len = DATAGRAM;
	
	buffers->primary = new WAVEHDR;
	buffers->secondary = new WAVEHDR;
	buffers->tertiary = new WAVEHDR;

	buffers->buf = new char[BUFFER];

	buffers->primary->lpData = buffers->buf;
	buffers->primary->dwBufferLength = BUFFER;

	memcpy_s(buffers->secondary, sizeof(*(buffers->secondary)), \
		buffers->primary, sizeof(*(buffers->primary)));
	memcpy_s(buffers->tertiary, sizeof(*(buffers->tertiary)), \
		buffers->primary, sizeof(*(buffers->primary)));

	result = waveOutPrepareHeader(buffers->waveout, buffers->primary, sizeof(*(buffers->primary)));
	result = waveOutPrepareHeader(buffers->waveout, buffers->secondary, sizeof(*(buffers->secondary)));
	result = waveOutPrepareHeader(buffers->waveout, buffers->tertiary, sizeof(*(buffers->tertiary)));
	if (result != MMSYSERR_NOERROR)
	{
		cerr << "Multicast: Error preparing headers (" << result << ")" << endl;
		return false;
	}

	result = waveOutWrite(buffers->waveout, buffers->primary, sizeof(*(buffers->primary)));
	result = waveOutWrite(buffers->waveout, buffers->secondary, sizeof(*(buffers->secondary)));
	result = waveOutWrite(buffers->waveout, buffers->tertiary, sizeof(*(buffers->tertiary)));
	if (result != MMSYSERR_NOERROR)
	{
		cerr << "Multicast: Error preparing headers (" << result << ")" << endl;
		return false;
	}

	return true;
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: RecvMultiThread
--
-- DATE: March 12, 2015
--
-- REVISIONS: Created March 10, 2015
--
-- DESIGNER: Michael Chimick
--
-- PROGRAMMER: Michael Chimick
--
-- INTERFACE: DWORD WINAPI RecvMultiThread(LPVOID parameter)
--
-- RETURNS: DWORD
--
-- NOTES:
--
--
----------------------------------------------------------------------------------------------------------------------*/
DWORD WINAPI RecvMultiThread(LPVOID parameter)
{
	WSARecv(socketInfo->socket, &(socketInfo->datagram), 1, &(socketInfo->bytesRECV), 0, &(socketInfo->overlapped), RecvMulti);
	while (true)
	{
		SleepEx(INFINITE, TRUE);
	}

	return 0;
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: RecvMulti
--
-- DATE: March 12, 2015
--
-- REVISIONS: Created March 10, 2015
--
-- DESIGNER: Michael Chimick
--
-- PROGRAMMER: Michael Chimick
--
-- INTERFACE: void CALLBACK RecvMulti(DWORD error, DWORD bytesTransferred, LPWSAOVERLAPPED overlapped, DWORD flags)
--
-- RETURNS: void
--
-- NOTES:
--
--
----------------------------------------------------------------------------------------------------------------------*/
void CALLBACK RecvMulti(DWORD error, DWORD bytesTransferred, LPWSAOVERLAPPED overlapped, DWORD flags)
{
	unsigned int spaceLeft;

	spaceLeft = (BUFFER - buffers->pos);
	if (socketInfo->bytesRECV > spaceLeft)
	{
		memcpy(buffers->buf + buffers->pos, socketInfo->datagram.buf, spaceLeft);
		buffers->pos = 0;
		socketInfo->bytesRECV -= spaceLeft;
		socketInfo->datagram.buf += spaceLeft;
	}
	memcpy(buffers->buf + buffers->pos, socketInfo->datagram.buf, socketInfo->bytesRECV);
	buffers->pos += socketInfo->bytesRECV;
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

void CALLBACK MultiWaveCallback(HWAVEOUT hWave, UINT uMsg, DWORD dwUser, DWORD dw1, DWORD dw2)
{
	MMRESULT result;

	if (uMsg == WOM_DONE)
	{
		result = waveOutWrite(buffers->waveout, (LPWAVEHDR)dw1, sizeof(*((LPWAVEHDR)dw1)));
		if (result != MMSYSERR_NOERROR)
			cout << "Multicast: playing buffer error (" << result << ")" << endl;
	}
}
