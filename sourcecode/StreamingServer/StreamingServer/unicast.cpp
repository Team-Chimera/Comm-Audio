#include "stdafx.h"
#include <cstring>
#include <mutex>
#include <iostream>
#include <vector>
#include <deque>
#include <string>
#include <vlc/vlc.h>
#include <vlc/libvlc.h>
#include "unicast.h"
#include "helper.h"
#include "music.h"
#include "newsession.h"

using namespace std;

//VLC instance objects
libvlc_instance_t *unicastInst;


//circular buffer
CircularBuffer unicastCircBuf;


//unicast variables
deque<UnicastClient> waitingClients;
mutex songMutex;

bool uniDone;
/*******************************************************************
** Function: startUnicast
**
** Date: April 6th, 2015
**
** Revisions:
**
**
** Designer: Rhea Lauzon
**
** Programmer: Rhea Lauzon
**
** Interface:
**			DWORD WINAPI startMulticastThread(LPVOID socket)
**				LPVOID socket -- The socket that requested unicast
**
**
** Returns:
**			true on success
**          false if error occurs
**
** Notes:
**  Beginning function of the unicast song playing.
*******************************************************************/
DWORD WINAPI startUnicast(LPVOID clientInfo)
{
	UnicastClient uc;

	// Create a datagram socket
	if ((uc.socket = socket(PF_INET, SOCK_DGRAM, 0)) == -1)
	{
		perror ("Can't create a socket");
		exit(1);
	}

	struct hostent *he;
	ClientData *cd = (ClientData *) clientInfo;

	   /* resolve hostname */
     if ((he = gethostbyname((cd->ip).c_str())) == NULL)
     {
         //error getting the host
         cerr << "Failed to retrieve host" << endl;
         exit(1);
     }

	// Bind an address to the socket
	memset ((char *)&uc.client, 0, sizeof(uc.client));
	uc.client.sin_family = AF_INET;
	uc.client.sin_port = htons(SERVER_UNICAST_PORT);
	memcpy((char *) &uc.client.sin_addr, he->h_addr, he->h_length);

	//place the new client into the deque
	waitingClients.push_back(uc);

	playUnicastSong(cd->song);

	cd->session->type = 1;
    ReleaseSemaphore(cd->session->transferCompleteSem, 1, 0);

	delete cd;
    return 0;
}

/** Make the music! **/
bool playUnicastSong(string songName)
{
	libvlc_media_player_t *unicastMediaPlayer;
	libvlc_media_t *unicastSong;

	int index = 0;
	char memoryOptions[256];
	sprintf_s(memoryOptions, VLC_OPTIONS, (long long int)(intptr_t)(void*) &handleUnicastStream, (long long int)(intptr_t)(void*) &prepareUnicastRender);
	const char* const vlcArgs[] = { "-I", "dummy", "--verbose=0", "--sout", memoryOptions};
	
	 //create an instance of libvlc
	if ((unicastInst = libvlc_new(sizeof(vlcArgs) / sizeof(vlcArgs[0]), vlcArgs)) == NULL)
	{
		cerr << "Failed to create libvlc instance." << endl;
		return false;
	}

	string song_to_play(song_dir + "/" + songName);
	unicastSong = libvlc_media_new_path(unicastInst, song_to_play.c_str());

	//load the song
	if (unicastSong == NULL)
	{
		cerr << "failed to load the song." << endl;
		return false;
	}

	//lock the mutex while another client has it
	songMutex.lock();

		
	//create a media player
	unicastMediaPlayer = libvlc_media_player_new_from_media(unicastSong);


	//Begin playing the music
	libvlc_media_release(unicastSong);

	//starts the process of streaming the audio (calls pre-render then handleStream)
	libvlc_media_player_play(unicastMediaPlayer);

	while (!libvlc_media_player_is_playing(unicastMediaPlayer))
	{
	// Wait for the song to start
	}

	//sleep constantly till the song finishes
	while (!uniDone && libvlc_media_player_is_playing(unicastMediaPlayer))
	{
		Sleep(1000);
	}

	libvlc_media_player_release(unicastMediaPlayer);

	//remove the finished client from the deque
	waitingClients.pop_front();

	//send a message to the client saying the unicast is over
	songMutex.unlock();


	return true;
}


/*****************************************************************
** Function: unicastAudioCleanup
**
** Date: March 30th, 2015
**
** Revisions:
**
**
** Designer: Rhea Lauzon
**
** Programmer: Rhea Lauzon
**
** Interface:
**			void unicastAudioCleanup()
**
** Returns:
**          void
**
** Notes:
** Removes the libVLC structures for sending data.
**
*******************************************************************/
void unicastAudioCleanup()
{
    libvlc_release(unicastInst);

}

/*****************************************************************
** Function: handleUncicastStream
**
** Date: March 28th, 2015
**
** Revisions:
**
**
** Designer: Rhea Lauzon
**
** Programmer: Rhea Lauzon
**
** Interface:
**			void handleUnicastStream(void* p_audio_data, uint8_t* p_pcm_buffer, 
**			unsigned int channels, unsigned int rate, unsigned int nb_samples,
**			unsigned int bits_per_sample, size_t size, int64_t pts)
**				void * p_audio_data -- Unused parameter
**				uint8_t * p_pcm_buffer -- pointer to location of the buffer
**				unsigned int channels -- Number of channels in use
**				unsigned int rate -- Bitrate
**				unsigned int nb_samples -- Samples/second of audio
**				unsigned int bits_per_sample -- Number of bits/sample (depth-wise)
**				size_t size -- Length of the buffer
**				int64_t pts -- Unused parameter
**				
**
** Returns:
**          void
**
** Notes:
** Handler for when the audio is streamed to write data
**
*******************************************************************/
void handleUnicastStream(void* p_audio_data, uint8_t* p_pcm_buffer, unsigned int channels, 
				  unsigned int rate, unsigned int nb_samples, unsigned int bits_per_sample, size_t size, int64_t pts)
{
	char *buffer;
	int dataSize = size;
	int messageSize;
	int dataSent = 0;

 

	// While we have data to write
	while (dataSize > 0)
	{
		// Set the size of the next message to send
		if (dataSize > MESSAGE_SIZE)
		{
			messageSize = MESSAGE_SIZE;
		}
		else
		{
			messageSize = dataSize;
		}

		// Write the data to the circular buffer
		buffer = new char[dataSize];
		memcpy(buffer, p_pcm_buffer + dataSent, messageSize);
		
		//place the data into the circular buffer		
		for (int i = 0; i < messageSize; i++)
		{
			unicastCircBuf.buf[unicastCircBuf.pos] = buffer[i];
			if(unicastCircBuf.pos == BUFFER_SIZE - 1)
			{
				unicastCircBuf.pos = 0;
			}
			else
			{
				unicastCircBuf.pos++;
			}
		}
		
		//update the sizing
		dataSize -= messageSize;
		dataSent += messageSize;

		
		//send over UDP
		if(sendto(waitingClients.front().socket, buffer, MESSAGE_SIZE, 0,(struct sockaddr *)&(waitingClients.front().client), sizeof(waitingClients.front().client)) <= 0)
		{
			perror ("sendto error");
			closesocket(waitingClients.front().socket);
			WSACleanup();
			return;
		}

		//remove the char array
		delete [] buffer;

	}

	// Free the temporary stream buffer
	free(p_pcm_buffer);
}


/*****************************************************************
** Function: prepareUnicastRender
**
** Date: March 28th, 2015
**
** Revisions:
**
**
** Designer: Rhea Lauzon
**
** Programmer: Rhea Lauzon
**
** Interface:
**			void prepareUnicastRender(void* p_audio_data, 
**			uint8_t** pp_pcm_buffer , size_t size)
**				void * p_audio_data -- unusued parameter
**				uint8_t ** pp_pcm_buffer -- Location of the audio stream
**				size_t size -- Size of the buffer
**
**				
**
** Returns:
**          void
**
** Notes:
** Called before audio streaming to allocate memory for the buffer
**
*******************************************************************/
void prepareUnicastRender(void* p_audio_data, uint8_t** pp_pcm_buffer , size_t size)
{
	// Allocate memory to the buffer
	*pp_pcm_buffer = (uint8_t*) malloc(size);
}
