#include "multicast.h"

void InitializeMulticastData()
{
	CreateSemaphoreEx(NULL, BUFFER / DATAGRAM, BUFFER / DATAGRAM, "multiBuffer\0", 0, SEMAPHORE_MODIFY_STATE);
	CreateSemaphoreEx(NULL, 1, 1, "multiPut\0", 0, SEMAPHORE_MODIFY_STATE);
	CreateSemaphoreEx(NULL, 0, 1, "multiUse\0", 0, SEMAPHORE_MODIFY_STATE);
}

void JoinMulticast(SOCKET multicast, in_addr group)
{
	struct ip_mreq addr;
	DWORD recvThread;
	DWORD playThread;

	addr.imr_multiaddr = group;
	// addr.imr_interface = inet_addr("0.0.0.0"); // get the local address
	setsockopt(multicast, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *) &addr, sizeof(addr));

	CreateThread(NULL, 0, RecvMultiThread, NULL, 0, &recvThread);
	CreateThread(NULL, 0, PlayMultiThread, NULL, 0, &recvThread);
}

DWORD WINAPI RecvMultiThread(LPVOID parameter)
{
	while (true)
	{
		SleepEx(INFINITE, TRUE);
		return 0; // remove when this is no longer an infinite loop
	}

	return 0;
}

// if we just recieved data, where is it stored?...
// Do I have to use globals?
void CALLBACK RecvMulti(DWORD error, DWORD bytesTransferred, LPWSAOVERLAPPED overlapped, DWORD flags)
{
	HANDLE semaBuf = OpenSemaphore(SEMAPHORE_MODIFY_STATE, TRUE, "multiBuffer\0");
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

DWORD WINAPI PlayMultiThread(LPVOID parameter)
{
	while (true)
	{
		// PlayMulti();
		return 0; // remove when this is no longer an infinite loop
	}

	return 0;
}

void PlayMulti(SemaFunc * semaStruct)
{
	char datagram[DATAGRAM]; // remember to change this from char

	HANDLE semaBuf = OpenSemaphore(SEMAPHORE_MODIFY_STATE, TRUE, "multiBuffer");
	HANDLE semaPut = OpenSemaphore(SEMAPHORE_MODIFY_STATE, TRUE, "multiPut");
	HANDLE semaUse = OpenSemaphore(SEMAPHORE_MODIFY_STATE, TRUE, "multiUse");

	// wait for semaUse
	WaitForSingleObjectEx(semaUse, INFINITE, TRUE);
	// wait for semaPut
	WaitForSingleObjectEx(semaPut, INFINITE, TRUE);

	// pull data from buffer
	for (int i = 0; i < DATAGRAM; i++) // remember to change this from char
	{
		if (semaStruct->use >= BUFFER) semaStruct->use = 0;
		datagram[i] = semaStruct->buffer[semaStruct->use];
		semaStruct->buffer[semaStruct->use] = '\0';
		semaStruct->use++;
	}

	// signal semaPut
	ReleaseSemaphore(semaPut, 1, NULL);
	// signal semaBuf
	ReleaseSemaphore(semaBuf, 1, NULL);

	OutputSpeakers(datagram);
}

void OutputSpeakers(char data[]) // remember to change this from char
{
	// output out the local speakers
}