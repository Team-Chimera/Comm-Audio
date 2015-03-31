#include <cstdlib>
#include <Windows.h>
#include <mmsystem.h>
#include <iostream>
#include <vlc/vlc.h>
#include <string>
#include <vlc/libvlc.h>
#include "music.h"

//circular buffer
CircularBuffer circBuf;

//media output device
HWAVEOUT output;
using namespace std;

//VLC instance objects
libvlc_instance_t *inst;
libvlc_media_player_t *mediaPlayer;
SOCKET sd;
struct	sockaddr_in server;
int	client_len;

int main(int argc, char * argv[])
{
	//create UDP socket
	WSADATA stWSAData;
	WORD wVersionRequested = MAKEWORD (2,2);
	
	// Initialize the DLL with version Winsock 2.2
	WSAStartup(wVersionRequested, &stWSAData ) ;

	// Create a datagram socket
	if ((sd = socket (PF_INET, SOCK_DGRAM, 0)) == -1)
	{
		perror ("Can't create a socket");
		exit(1);
	}

	    struct hostent *he;

    //convert the QString to a string
    string IP = "localhost";


     /* resolve hostname */
     if ((he = gethostbyname(IP.c_str())) == NULL)
     {
         //error getting the host
         cerr << "Failed to retrieve host" << endl;
         exit(1);
     }

	// Bind an address to the socket
	memset ((char *)&server, 0, sizeof(server));
	server.sin_family = AF_INET;
	server.sin_port = htons(9003);
  memcpy((char *) &server.sin_addr, he->h_addr, he->h_length);


	//initialize the buffer position to 0
	circBuf.pos = 0;

	char memoryOptions[256];

	//write the options ot the char array
	 sprintf(memoryOptions, VLC_OPTIONS, (long long int)(intptr_t)(void*) &handleStream, (long long int)(intptr_t)(void*) &prepareRender);
	 const char* const vlcArgs[] = { "-I", "dummy", "--verbose=0", "--sout", memoryOptions};
	
	 //create an instance of libvlc
	if ((inst = libvlc_new(sizeof(vlcArgs) / sizeof(vlcArgs[0]), vlcArgs)) == NULL)
	{
		cerr << "Failed to create libvlc instance." << endl;
		return -1;
	}

	//load a song from command line else the test song
	libvlc_media_t *song;
	if (argc > 1)
	{
		song =  libvlc_media_new_path(inst, argv[1]);
	}
	else
	{
		song = libvlc_media_new_path(inst, "test.mp3");
	}

	//load the song
	if (song == NULL)
	{
		cerr << "failed to load the song." << endl;
		exit(1);
	}

	//create a media player
	mediaPlayer = libvlc_media_player_new_from_media(song);

	//Begin playing the music
	libvlc_media_release(song);

	//starts the process of streaming the audio (calls pre-render then handleStream)
	libvlc_media_player_play(mediaPlayer);

	// Create the output thread
	//aka receive
	//CreateThread(NULL, 0, playSong, NULL, 0, NULL);

	// Wait for any character to quit
	getchar();

	audioCleanup();

	//clear socket
	closesocket(sd);
	WSACleanup();
	
	return 0;


}

/*****************************************************************
** Function: audioCleanup
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
**			void audioCleanup()
**
** Returns:
**          void
**
** Notes:
** Removes the libVLC structures for sending data.
**
*******************************************************************/
void audioCleanup()
{
	libvlc_media_player_release(mediaPlayer);
    libvlc_release(inst);

}

/*****************************************************************
** Function: playSong
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
**			DWORD WINAPI playSong(LPVOID arg)
**				LPVOID arg -- Thread arguments
**
** Returns:
**          DWORD -- -1 on failure; 0 on success
**
** Notes:
** Plays the received song in a thread.
**
*******************************************************************/
DWORD WINAPI playSong(LPVOID arg)
{
	//create the wave header
	WAVEFORMATEX wavFormat;
	
	//create a number of buffers; in this case 3
	LPWAVEHDR audioBuffers[NUM_OUTPUT_BUFFERS];

	//set up the format
	wavFormat.nSamplesPerSec = 44100;
	wavFormat.wBitsPerSample = 16;
	wavFormat.nChannels = 2;
	wavFormat.cbSize = 0;
	wavFormat.wFormatTag = WAVE_FORMAT_PCM;
	wavFormat.nBlockAlign = wavFormat.nChannels * (wavFormat.wBitsPerSample / 8);
	wavFormat.nAvgBytesPerSec = wavFormat.nSamplesPerSec * wavFormat.wBitsPerSample;
	
	//open the media device for streaming to
	if (waveOutOpen(&output, WAVE_MAPPER, &wavFormat, (DWORD) waveCallback, NULL, CALLBACK_FUNCTION) != MMSYSERR_NOERROR)
	{
		cerr << "Failed to open output device." << endl;
		return -1;
	}

	// Prepare the wave headers
	for (int i = 0; i < NUM_OUTPUT_BUFFERS; i++)
	{
		//malloc and clear the memory
		audioBuffers[i] = (LPWAVEHDR) malloc(sizeof(WAVEHDR));
		ZeroMemory(audioBuffers[i], sizeof(WAVEHDR));

		//update the their ubffers to a position in the tripple buffer
		audioBuffers[i]->lpData = circBuf.buf;
		audioBuffers[i]->dwBufferLength = BUFFER_SIZE;

		// Create the header
		if (waveOutPrepareHeader(output, audioBuffers[i], sizeof(WAVEHDR)) != MMSYSERR_NOERROR)
		{
			cerr << "Failed to create output header." << endl;
			exit(1);
		}
	}

		//write audio to the buffer
		cout << "I am ready to play music!" << endl;
		
		//wait until we have two messages worth of data; this avoids crackle
		while(circBuf.pos < MESSAGE_SIZE * 2)
		{
			//wait for the buffer to be ready
		}

		for (int i = 0; i < NUM_OUTPUT_BUFFERS; i++)
		{
			waveOutWrite(output, audioBuffers[i], sizeof(WAVEHDR));
		}

		return 0;

}

/*****************************************************************
** Function: waveCallback
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
**			void CALLBACK waveCallback(HWAVEOUT hWave, UINT uMsg, 
**								DWORD dwUser, DWORD dw1, DWORD dw2)
**				HWAVEOUT hWave -- handle to the output device
**				UINT uMsg -- message sent to the callback
**				DWORD dwUser -- Unused parameter
**				DWORD dw1 -- the header used for this audio output
**				DWORD dw2 -- Unused parameter
**				
**
** Returns:
**          void
**
** Notes:
** Plays the audio without skipping issues.
**
*******************************************************************/
void CALLBACK waveCallback(HWAVEOUT hWave, UINT uMsg, DWORD dwUser, DWORD dw1, DWORD dw2)
{
	if (uMsg == WOM_DONE)
	{
		if (waveOutWrite(output, (LPWAVEHDR) dw1, sizeof(WAVEHDR)) != MMSYSERR_NOERROR)
		{
			cerr << "Failed to play audio." << endl;
			exit(1);
		}
	}
}



/*****************************************************************
** Function: handleStream
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
**			void handleStream(void* p_audio_data, uint8_t* p_pcm_buffer, 
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
void handleStream(void* p_audio_data, uint8_t* p_pcm_buffer, unsigned int channels, 
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
			circBuf.buf[circBuf.pos] = buffer[i];
			if(circBuf.pos == BUFFER_SIZE - 1)
			{
				circBuf.pos = 0;
			}
			else
			{
				circBuf.pos++;
			}
		}
		
		//update the sizing
		dataSize -= messageSize;
		dataSent += messageSize;

				//send over UDP
		if(sendto(sd, buffer, MESSAGE_SIZE, 0,(struct sockaddr *)&server, sizeof(server)) <= 0)
		{
			perror ("sendto error");
			closesocket(sd);
			WSACleanup();
			exit(0);
		}

		//remove the char array
		delete [] buffer;

	}

	// Free the temporary stream buffer
	free(p_pcm_buffer);
}


/*****************************************************************
** Function: prepareRender
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
**			void prepareRender(void* p_audio_data, 
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
void prepareRender(void* p_audio_data, uint8_t** pp_pcm_buffer , size_t size)
{
	// Allocate memory to the buffer
	*pp_pcm_buffer = (uint8_t*) malloc(size);
}
