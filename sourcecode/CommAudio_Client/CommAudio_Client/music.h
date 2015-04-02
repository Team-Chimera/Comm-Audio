#ifndef MUSIC_H
#define MUSIC_H

#define VLC_OPTIONS "#transcode{acodec=s16l,samplerate=44100,channels=2}:smem{audio-postrender-callback=%lld,audio-prerender-callback=%lld}"

/* The number of output buffers for streaming audio */
#define NUM_OUTPUT_BUFFERS 3

/* The size of a message received from the socket */
#define MESSAGE_SIZE 4096
#define PRE_BUFFER_SIZE (5 * MESSAGE_SIZE)

/* The size of the circular audio buffer */
#define MUSIC_BUFFER_SIZE (MESSAGE_SIZE * 20)

/* The settings for wave format structs */
#define SAMPLES_PER_SECOND 44100
#define BITS_PER_SAMPLE    16
#define CHANNELS           2

typedef struct CircularBuffer
{
    char buf[MUSIC_BUFFER_SIZE];
    int pos;
} CircularBuffer;


#endif // MUSIC_H
