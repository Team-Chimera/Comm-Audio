/*------------------------------------------------------------------------------------------------------------------
-- SOURCE FILE: Voice.cpp
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

#include "voiceChat.h"
#include <iostream>

using std::cout;
using std::cerr;
using std::endl;

SOCKET_INFORMATION * recvSocketInfo;
SOCKET_INFORMATION * sendSocketInfo;
Semaphores * recvSemaphores;
Semaphores * sendSemaphores;
HANDLE voiceThreads[4];

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: InitializeVoiceData
--
-- DATE: March 12, 2015
--
-- REVISIONS: Created March 10, 2015
--
-- DESIGNER: Michael Chimick
--
-- PROGRAMMER: Michael Chimick
--
-- INTERFACE: void InitializeVoiceData()
--
-- RETURNS: void
--
-- NOTES:
-- Initializes data needed for Voiceing
--
----------------------------------------------------------------------------------------------------------------------*/
void InitVoiceData()
{
	recvSocketInfo = new SOCKET_INFORMATION;
	recvSocketInfo->cBuffer = new CircularBuffer(BUFFER);
	recvSocketInfo->datagram.buf = new char[DATAGRAM];
	recvSocketInfo->datagram.len = DATAGRAM;

	sendSocketInfo = new SOCKET_INFORMATION;
	sendSocketInfo->cBuffer = new CircularBuffer(BUFFER);
	sendSocketInfo->datagram.buf = new char[DATAGRAM];
	sendSocketInfo->datagram.len = DATAGRAM;

	recvSemaphores = new Semaphores;

	sendSemaphores = new Semaphores;
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: JoinVoice
--
-- DATE: March 12, 2015
--
-- REVISIONS: Created March 10, 2015
--
-- DESIGNER: Michael Chimick
--
-- PROGRAMMER: Michael Chimick
--
-- INTERFACE: void JoinVoice(SOCKET Voice, in_addr group)
--
-- RETURNS: void
--
-- NOTES:
-- Joins the Voice session, and starts Voice processing
--
----------------------------------------------------------------------------------------------------------------------*/
void JoinVoice(SOCKET recv, SOCKET send, in_addr group, in_addr local)
{
	DWORD recvThread;
	DWORD playThread;
	DWORD sendThread;
	DWORD recordThread;

	if (recvSocketInfo == NULL) return;
	if (sendSocketInfo == NULL) return;

	recvSemaphores->semaBuf = CreateSemaphoreEx(NULL, BUFFER / DATAGRAM, BUFFER / DATAGRAM, NULL, 0, SEMAPHORE_MODIFY_STATE);
	recvSemaphores->semaIn = CreateSemaphoreEx(NULL, 1, 1, NULL, 0, SEMAPHORE_MODIFY_STATE);
	recvSemaphores->semaOut = CreateSemaphoreEx(NULL, 0, 1, NULL, 0, SEMAPHORE_MODIFY_STATE);

	sendSemaphores->semaBuf = CreateSemaphoreEx(NULL, BUFFER / DATAGRAM, BUFFER / DATAGRAM, NULL, 0, SEMAPHORE_MODIFY_STATE);
	sendSemaphores->semaIn = CreateSemaphoreEx(NULL, 1, 1, NULL, 0, SEMAPHORE_MODIFY_STATE);
	sendSemaphores->semaOut = CreateSemaphoreEx(NULL, 0, 1, NULL, 0, SEMAPHORE_MODIFY_STATE);

	recvSocketInfo->socket = recv;
	sendSocketInfo->socket = send;

	//socketInfo->addr.imr_multiaddr = group;
	//socketInfo->addr.imr_interface = local;
	//setsockopt(socketInfo->socket, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *)&(socketInfo->addr), sizeof(socketInfo->addr));

	voiceThreads[0] = CreateThread(NULL, 0, RecvVoiceThread, NULL, 0, &recvThread);
	voiceThreads[1] = CreateThread(NULL, 0, PlayVoiceThread, NULL, 0, &playThread);
	voiceThreads[2] = CreateThread(NULL, 0, SendVoiceThread, NULL, 0, &sendThread);
	voiceThreads[3] = CreateThread(NULL, 0, RecordVoiceThread, NULL, 0, &recordThread);
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: DropVoice
--
-- DATE: March 12, 2015
--
-- REVISIONS: Created March 10, 2015
--
-- DESIGNER: Michael Chimick
--
-- PROGRAMMER: Michael Chimick
--
-- INTERFACE: void DropVoice()
--
-- RETURNS: void
--
-- NOTES:
-- Drops the Voice session, and stops Voice processing
--
----------------------------------------------------------------------------------------------------------------------*/
void DropVoice()
{
	TerminateThread(voiceThreads[0], 0);
	TerminateThread(voiceThreads[1], 0);
	TerminateThread(voiceThreads[2], 0);
	TerminateThread(voiceThreads[3], 0);

	CloseHandle(recvSemaphores->semaBuf);
	CloseHandle(recvSemaphores->semaIn);
	CloseHandle(recvSemaphores->semaOut);

	CloseHandle(sendSemaphores->semaBuf);
	CloseHandle(sendSemaphores->semaIn);
	CloseHandle(sendSemaphores->semaOut);

	//setsockopt(socketInfo->socket, IPPROTO_IP, IP_DROP_MEMBERSHIP, (char *)&(socketInfo->addr), sizeof(socketInfo->addr));
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
	// wait for semaBuf
	WaitForSingleObjectEx(recvSemaphores->semaBuf, INFINITE, TRUE);
	// wait for semaIn
	WaitForSingleObjectEx(recvSemaphores->semaIn, INFINITE, TRUE);

	// In data into buffer
	if (recvSocketInfo->cBuffer->In((byte*)recvSocketInfo->datagram.buf, DATAGRAM) == false)
		cerr << "RecvVoice: buffer insertion error" << endl;
	else
		cout << "RecvVoice: buffer insertion success" << endl;

	// signal semaIn
	ReleaseSemaphore(recvSemaphores->semaIn, 1, NULL);
	// signal semaOut
	ReleaseSemaphore(recvSemaphores->semaOut, 1, NULL);
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: PlayVoiceThread
--
-- DATE: March 12, 2015
--
-- REVISIONS: Created March 10, 2015
--
-- DESIGNER: Michael Chimick
--
-- PROGRAMMER: Michael Chimick
--
-- INTERFACE: DWORD WINAPI PlayVoiceThread(LPVOID parameter)
--
-- RETURNS: DWORD
--
-- NOTES:
--
--
----------------------------------------------------------------------------------------------------------------------*/
DWORD WINAPI PlayVoiceThread(LPVOID parameter)
{
	while (true)
	{
		PlayVoice();
	}

	return 0;
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: PlayVoice
--
-- DATE: March 12, 2015
--
-- REVISIONS: Created March 10, 2015
--
-- DESIGNER: Michael Chimick
--
-- PROGRAMMER: Michael Chimick
--
-- INTERFACE: void PlayVoice(BufControl * bCont)
--
-- RETURNS: void
--
-- NOTES:
--
--
----------------------------------------------------------------------------------------------------------------------*/
void PlayVoice()
{
	byte datagram[DATAGRAM];

	// wait for semaOut
	WaitForSingleObjectEx(recvSemaphores->semaOut, INFINITE, TRUE);
	// wait for semaIn
	WaitForSingleObjectEx(recvSemaphores->semaIn, INFINITE, TRUE);

	// pull data from buffer
	if (recvSocketInfo->cBuffer->Out(datagram, DATAGRAM) == false)
		cerr << "PlayVoice: buffer removal error" << endl;
	else
		cout << "PlayVoice: buffer removal success" << endl;

	// signal semaIn
	ReleaseSemaphore(recvSemaphores->semaIn, 1, NULL);
	// signal semaBuf
	ReleaseSemaphore(recvSemaphores->semaBuf, 1, NULL);

	VoiceOutputSpeakers(datagram, DATAGRAM);
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
void VoiceOutputSpeakers(byte data[], int size)
{
	// out the local speakers
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
DWORD WINAPI RecordVoiceThread(LPVOID parameter)
{
	while (true)
	{
		RecordVoice();
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
void RecordVoice()
{
	// wait for semaBuf
	WaitForSingleObjectEx(sendSemaphores->semaBuf, INFINITE, TRUE);
	// wait for semaIn
	WaitForSingleObjectEx(sendSemaphores->semaIn, INFINITE, TRUE);

	// In data into buffer
	if (sendSocketInfo->cBuffer->In((byte*)sendSocketInfo->datagram.buf, DATAGRAM) == false)
		cerr << "RecvVoice: buffer insertion error" << endl;
	else
		cout << "RecvVoice: buffer insertion success" << endl;

	// signal semaIn
	ReleaseSemaphore(sendSemaphores->semaIn, 1, NULL);
	// signal semaOut
	ReleaseSemaphore(sendSemaphores->semaOut, 1, NULL);
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: PlayVoiceThread
--
-- DATE: March 12, 2015
--
-- REVISIONS: Created March 10, 2015
--
-- DESIGNER: Michael Chimick
--
-- PROGRAMMER: Michael Chimick
--
-- INTERFACE: DWORD WINAPI PlayVoiceThread(LPVOID parameter)
--
-- RETURNS: DWORD
--
-- NOTES:
--
--
----------------------------------------------------------------------------------------------------------------------*/
DWORD WINAPI SendVoiceThread(LPVOID parameter)
{
	while (true)
	{
		SendVoice();
	}

	return 0;
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: PlayVoice
--
-- DATE: March 12, 2015
--
-- REVISIONS: Created March 10, 2015
--
-- DESIGNER: Michael Chimick
--
-- PROGRAMMER: Michael Chimick
--
-- INTERFACE: void PlayVoice(BufControl * bCont)
--
-- RETURNS: void
--
-- NOTES:
--
--
----------------------------------------------------------------------------------------------------------------------*/
void SendVoice()
{
	byte datagram[DATAGRAM];

	// wait for semaOut
	WaitForSingleObjectEx(sendSemaphores->semaOut, INFINITE, TRUE);
	// wait for semaIn
	WaitForSingleObjectEx(sendSemaphores->semaIn, INFINITE, TRUE);

	// pull data from buffer
	if (sendSocketInfo->cBuffer->Out(datagram, DATAGRAM) == false)
		cerr << "PlayVoice: buffer removal error" << endl;
	else
		cout << "PlayVoice: buffer removal success" << endl;

	// signal semaIn
	ReleaseSemaphore(sendSemaphores->semaIn, 1, NULL);
	// signal semaBuf
	ReleaseSemaphore(sendSemaphores->semaBuf, 1, NULL);

	VoiceOutputSpeakers(datagram, DATAGRAM);
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
