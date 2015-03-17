#include "voiceChat.h"

void InitializeMulticastData()
{
	CreateSemaphoreEx(NULL, BUFFER / DATAGRAM, BUFFER / DATAGRAM, "voiceBuf\0", 0, SEMAPHORE_MODIFY_STATE);
	CreateSemaphoreEx(NULL, 1, 1, "voicePut\0", 0, SEMAPHORE_MODIFY_STATE);
	CreateSemaphoreEx(NULL, 0, 1, "voiceUse\0", 0, SEMAPHORE_MODIFY_STATE);
}


void StartVoiceChat()
{
	// sending thread
	// receiving thread
	// recording thread

	// playing function
}

void VoiceRecord()
{
	HANDLE semaBuf = OpenSemaphore(SEMAPHORE_MODIFY_STATE, TRUE, "voiceBuf\0");
	HANDLE semaPut = OpenSemaphore(SEMAPHORE_MODIFY_STATE, TRUE, "voicePut\0");
	HANDLE semaUse = OpenSemaphore(SEMAPHORE_MODIFY_STATE, TRUE, "voiceUse\0");

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

void VoiceSend()
{
	HANDLE semaBuf = OpenSemaphore(SEMAPHORE_MODIFY_STATE, TRUE, "voiceBuffer");
	HANDLE semaPut = OpenSemaphore(SEMAPHORE_MODIFY_STATE, TRUE, "voicePut");
	HANDLE semaUse = OpenSemaphore(SEMAPHORE_MODIFY_STATE, TRUE, "voiceUse");

	// wait for semaUse
	WaitForSingleObjectEx(semaUse, INFINITE, TRUE);
	// wait for semaPut
	WaitForSingleObjectEx(semaPut, INFINITE, TRUE);

	// pull data from buffer

	// signal semaPut
	ReleaseSemaphore(semaPut, 1, NULL);
	// signal semaBuf
	ReleaseSemaphore(semaBuf, 1, NULL);
}

void VoiceRecv()
{
	HANDLE semaBuf = OpenSemaphore(SEMAPHORE_MODIFY_STATE, TRUE, "voiceBuf\0");
	HANDLE semaPut = OpenSemaphore(SEMAPHORE_MODIFY_STATE, TRUE, "voicePut\0");
	HANDLE semaUse = OpenSemaphore(SEMAPHORE_MODIFY_STATE, TRUE, "voiceUse\0");

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

void VoicePlay()
{
	HANDLE semaBuf = OpenSemaphore(SEMAPHORE_MODIFY_STATE, TRUE, "voiceBuffer");
	HANDLE semaPut = OpenSemaphore(SEMAPHORE_MODIFY_STATE, TRUE, "voicePut");
	HANDLE semaUse = OpenSemaphore(SEMAPHORE_MODIFY_STATE, TRUE, "voiceUse");

	// wait for semaUse
	WaitForSingleObjectEx(semaUse, INFINITE, TRUE);
	// wait for semaPut
	WaitForSingleObjectEx(semaPut, INFINITE, TRUE);

	// pull data from buffer

	// signal semaPut
	ReleaseSemaphore(semaPut, 1, NULL);
	// signal semaBuf
	ReleaseSemaphore(semaBuf, 1, NULL);
}
