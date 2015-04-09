#ifndef UNICAST
#define UNICAST

#include <WinSock2.h>
#include <ws2tcpip.h>
#include <cstdio>
#include <mmsystem.h>

#define SERVER_UNICAST_PORT 6698

struct UnicastClient
{
	SOCKET socket;
	struct sockaddr_in client;
};

typedef struct UnicastClient UnicastClient;

DWORD WINAPI startUnicast(LPVOID);
void prepareUnicastRender(void* , uint8_t** , size_t);
void handleUnicastStream(void* p_audio_data, uint8_t* p_pcm_buffer, unsigned int channels, 
				  unsigned int rate, unsigned int nb_samples, unsigned int bits_per_sample, size_t size, int64_t pts);

bool playUnicastSong(std::string);

void unicastAudioCleanup();

#endif