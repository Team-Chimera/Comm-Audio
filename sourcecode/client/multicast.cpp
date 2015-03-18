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

SOCKET_INFORMATION * socketInfo;
Semaphores * semaphores;
HANDLE threads[2];

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: InitializeMulticastData
--
-- DATE: March 12, 2015
--
-- REVISIONS: Created March 10, 2015
--
-- DESIGNER: Michael Chimick
--
-- PROGRAMMER: Michael Chimick
--
-- INTERFACE: void InitializeMulticastData()
--
-- RETURNS: void
--
-- NOTES:
-- Initializes data needed for multicasting
--
----------------------------------------------------------------------------------------------------------------------*/
void InitializeMulticastData()
{
    socketInfo = new SOCKET_INFORMATION;
    socketInfo->cBuffer = new CircularBuffer(BUFFER);
    socketInfo->datagram.buf = new char[DATAGRAM];
    socketInfo->datagram.len = DATAGRAM;

    semaphores = new Semaphores;
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
void JoinMulticast(SOCKET s, OVERLAPPED o, in_addr group, in_addr local)
{
	DWORD recvThread;
	DWORD playThread;

    if (socketInfo == NULL) return;

    semaphores->semaBuf = CreateSemaphoreEx(NULL, BUFFER / DATAGRAM, BUFFER / DATAGRAM, NULL, 0, SEMAPHORE_MODIFY_STATE);
    semaphores->semaIn = CreateSemaphoreEx(NULL, 1, 1, NULL, 0, SEMAPHORE_MODIFY_STATE);
    semaphores->semaOut = CreateSemaphoreEx(NULL, 0, 1, NULL, 0, SEMAPHORE_MODIFY_STATE);

    socketInfo->socket = s;
    socketInfo->overlapped = o;

    socketInfo->addr.imr_multiaddr = group;
    socketInfo->addr.imr_interface = local;
    setsockopt(socketInfo->socket, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *) &(socketInfo->addr), sizeof(socketInfo->addr));

    threads[0] = CreateThread(NULL, 0, RecvMultiThread, NULL, 0, &recvThread);
    threads[1] = CreateThread(NULL, 0, PlayMultiThread, NULL, 0, &playThread);
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
void DropMulticast()
{
    TerminateThread(thread[0], 0);
    TerminateThread(thread[1], 0);

    CloseHandle(semaphores->semaBuf);
    CloseHandle(semaphores->semaIn);
    CloseHandle(semaphores->semaOut);

    setsockopt(socketInfo->socket, IPPROTO_IP, IP_DROP_MEMBERSHIP, (char *) &(socketInfo->addr), sizeof(socketInfo->addr));
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
    WSARecv(socketInfo->socket, socketInfo->datagram, 1, &(socketInfo->bytesRECV), 0, &(socketInfo->overlapped), RecvMulti);
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
	// wait for semaBuf
	WaitForSingleObjectEx(semaBuf, INFINITE, TRUE);
    // wait for semaIn
    WaitForSingleObjectEx(semaIn, INFINITE, TRUE);

    // In data into buffer
    if (socketInfo->cBuffer->In((byte*)socketInfo->datagram, DATAGRAM) == false)
        cerr << "RecvMulti: buffer insertion error" << endl;
    else
        cout << "RecvMulti: buffer insertion success" << endl;

    // signal semaIn
    ReleaseSemaphore(semaIn, 1, NULL);
    // signal semaOut
    ReleaseSemaphore(semaOut, 1, NULL);
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: PlayMultiThread
--
-- DATE: March 12, 2015
--
-- REVISIONS: Created March 10, 2015
--
-- DESIGNER: Michael Chimick
--
-- PROGRAMMER: Michael Chimick
--
-- INTERFACE: DWORD WINAPI PlayMultiThread(LPVOID parameter)
--
-- RETURNS: DWORD
--
-- NOTES:
--
--
----------------------------------------------------------------------------------------------------------------------*/
DWORD WINAPI PlayMultiThread(LPVOID parameter)
{
	while (true)
	{
        PlayMulti();
	}

	return 0;
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: PlayMulti
--
-- DATE: March 12, 2015
--
-- REVISIONS: Created March 10, 2015
--
-- DESIGNER: Michael Chimick
--
-- PROGRAMMER: Michael Chimick
--
-- INTERFACE: void PlayMulti(BufControl * bCont)
--
-- RETURNS: void
--
-- NOTES:
--
--
----------------------------------------------------------------------------------------------------------------------*/
void PlayMulti()
{
    byte datagram[DATAGRAM];

    // wait for semaOut
    WaitForSingleObjectEx(semaOut, INFINITE, TRUE);
    // wait for semaIn
    WaitForSingleObjectEx(semaIn, INFINITE, TRUE);

	// pull data from buffer
    if (socket->cBuffer->Out(datagram, DATAGRAM) == false)
        cerr << "PlayMulti: buffer removal error" << endl;
    else
        cout << "PlayMulti: buffer removal success" << endl;

    // signal semaIn
    ReleaseSemaphore(semaIn, 1, NULL);
	// signal semaBuf
	ReleaseSemaphore(semaBuf, 1, NULL);

    OutInSpeakers(datagram);
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
void OutInSpeakers(byte data[]) // remember to change this from char
{
    // outIn out the local speakers
}
