#define VLC_OPTIONS "#transcode{acodec=s16l,samplerate=44100,channels=2}:smem{audio-postrender-callback=%lld,audio-prerender-callback=%lld}"

/* The number of output buffers for streaming audio */
#define NUM_OUTPUT_BUFFERS 3

/* The size of a message received from the socket */
#define MESSAGE_SIZE 512

/* The size of the circular audio buffer */
#define BUFFER_SIZE (MESSAGE_SIZE * 10)

typedef struct CircularBuffer
{
	char buf[BUFFER_SIZE];
	int pos;
} CircularBuffer;

//function prototpes
void prepareRender(void* p_audio_data, uint8_t** pp_pcm_buffer , size_t size); 
void handleStream(void* p_audio_data, uint8_t* p_pcm_buffer, unsigned int channels, 
				  unsigned int rate, unsigned int nb_samples, unsigned int bits_per_sample, size_t size, int64_t pts);
void audioCleanup();


