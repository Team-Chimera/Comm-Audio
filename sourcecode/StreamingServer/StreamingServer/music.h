#define VLC_OPTIONS "#transcode{acodec=s16l,samplerate=44100,channels=2}:smem{audio-postrender-callback=%lld,audio-prerender-callback=%lld}"

/* The number of output buffers for streaming audio */
#define NUM_OUTPUT_BUFFERS 3

/* The size of a message received from the socket */
#define MESSAGE_SIZE 512

/* The size of the circular audio buffer */
#define BUFFER_SIZE (MESSAGE_SIZE * 10)


/** Circular Buffer Struct **/
typedef struct CircularBuffer
{
	char buf[BUFFER_SIZE];
	int pos;
} CircularBuffer;


/* A struct containing metadata for a song */
typedef struct MetaData
{
	char *title;
	char *artist;
	char *album;
} MetaData;

//function prototpes
void prepareRender(void* p_audio_data, uint8_t** pp_pcm_buffer , size_t size); 
void handleStream(void* p_audio_data, uint8_t* p_pcm_buffer, unsigned int channels, 
				  unsigned int rate, unsigned int nb_samples, unsigned int bits_per_sample, size_t size, int64_t pts);
void audioCleanup();

void freeMetaData(MetaData *);
bool getMetaData(MetaData *, libvlc_media_t *);
void fetchMetaData(MetaData *);

