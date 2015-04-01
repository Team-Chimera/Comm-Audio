#ifndef MUSIC_H
#define MUSIC_H

#define VLC_OPTIONS "#transcode{acodec=s16l,samplerate=44100,channels=2}:smem{audio-postrender-callback=%lld,audio-prerender-callback=%lld}"

/* The number of output buffers for streaming audio */
#define NUM_OUTPUT_BUFFERS 3

/* The size of a message received from the socket */
#define MESSAGE_SIZE 4096

/* The size of the circular audio buffer */
#define MUSIC_BUFFER_SIZE (MESSAGE_SIZE * 20)

typedef struct CircularBuffer
{
    char buf[MUSIC_BUFFER_SIZE];
    int pos;
} CircularBuffer;


#endif // MUSIC_H
