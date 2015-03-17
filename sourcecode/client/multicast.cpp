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
	CreateSemaphoreEx(NULL, BUFFER / DATAGRAM, BUFFER / DATAGRAM, "multiBuf\0", 0, SEMAPHORE_MODIFY_STATE);
	CreateSemaphoreEx(NULL, 1, 1, "multiPut\0", 0, SEMAPHORE_MODIFY_STATE);
	CreateSemaphoreEx(NULL, 0, 1, "multiUse\0", 0, SEMAPHORE_MODIFY_STATE);
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
void JoinMulticast(SOCKET multicast, in_addr group, in_addr local)
{
	struct ip_mreq addr;
	DWORD recvThread;
	DWORD playThread;

	addr.imr_multiaddr = group;
    addr.imr_interface = local;
	setsockopt(multicast, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *) &addr, sizeof(addr));

	CreateThread(NULL, 0, RecvMultiThread, NULL, 0, &recvThread);
	CreateThread(NULL, 0, PlayMultiThread, NULL, 0, &recvThread);
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
	while (true)
	{
		SleepEx(INFINITE, TRUE);
		return 0; // remove when this is no longer an infinite loop
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
	HANDLE semaBuf = OpenSemaphore(SEMAPHORE_MODIFY_STATE, TRUE, "multiBuf\0");
	HANDLE semaPut = OpenSemaphore(SEMAPHORE_MODIFY_STATE, TRUE, "multiPut\0");
	HANDLE semaUse = OpenSemaphore(SEMAPHORE_MODIFY_STATE, TRUE, "multiUse\0");

	// wait for semaBuf
	WaitForSingleObjectEx(semaBuf, INFINITE, TRUE);
	// wait for semaPut
	WaitForSingleObjectEx(semaPut, INFINITE, TRUE);

	// put data into buffer

	// signal semaPut
	ReleaseSemaphore(semaPut, 1, NULL);
	// signal semaUse
	ReleaseSemaphore(semaUse, 1, NULL);
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
		// PlayMulti();
		return 0; // remove when this is no longer an infinite loop
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
void PlayMulti(BufControl * bCont)
{
	char datagram[DATAGRAM]; // remember to change this from char

    HANDLE semaBuf = OpenSemaphore(SEMAPHORE_MODIFY_STATE, TRUE, "multiBuf");
	HANDLE semaPut = OpenSemaphore(SEMAPHORE_MODIFY_STATE, TRUE, "multiPut");
	HANDLE semaUse = OpenSemaphore(SEMAPHORE_MODIFY_STATE, TRUE, "multiUse");

	// wait for semaUse
	WaitForSingleObjectEx(semaUse, INFINITE, TRUE);
	// wait for semaPut
	WaitForSingleObjectEx(semaPut, INFINITE, TRUE);

	// pull data from buffer
	for (int i = 0; i < DATAGRAM; i++) // remember to change this from char
	{
		if (bCont->use >= BUFFER) bCont->use = 0;
		datagram[i] = bCont->buffer[bCont->use];
		bCont->buffer[bCont->use] = '\0';
		bCont->use++;
	}

	// signal semaPut
	ReleaseSemaphore(semaPut, 1, NULL);
	// signal semaBuf
	ReleaseSemaphore(semaBuf, 1, NULL);

	OutputSpeakers(datagram);
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
void OutputSpeakers(char data[]) // remember to change this from char
{
	// output out the local speakers
}
