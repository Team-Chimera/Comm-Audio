#ifndef MUSIC_H
#define MUSIC_H


#include <stdint.h>

#define VLC_OPTIONS "#transcode{acodec=s16l,samplerate=44100,channels=2}:smem{audio-postrender-callback=%lld,audio-prerender-callback=%lld}"

/* The number of output buffers for streaming audio */
#define NUM_OUTPUT_BUFFERS 3

/* The size of a message received from the socket */
#define MESSAGE_SIZE 4096

/* The size of the circular audio buffer */
#define MUSIC_BUFFER_SIZE (MESSAGE_SIZE * 10)

typedef struct CircularBuffer
{
    char buf[MUSIC_BUFFER_SIZE];
    int pos;
} CircularBuffer;

//function prototpes
DWORD WINAPI playSong(LPVOID arg);
void CALLBACK waveCallback(HWAVEOUT hWave, UINT uMsg, DWORD dwUser, DWORD dw1, DWORD dw2);
void prepareRender(void* p_audio_data, uint8_t** pp_pcm_buffer , size_t size);
void handleStream(void* p_audio_data, uint8_t* p_pcm_buffer, unsigned int channels,
                  unsigned int rate, unsigned int nb_samples, unsigned int bits_per_sample, size_t size, int64_t pts);
void audioCleanup();



#endif // MUSIC_H
