#define CLIENT_UNICAST_PORT 6698
#define NOMINMAX

#include <mmsystem.h>
#include <stdint.h>

DWORD WINAPI unicastSong(LPVOID);
void receiveData();

//function prototpes
DWORD WINAPI playSong(LPVOID arg);
void CALLBACK waveCallback(HWAVEOUT hWave, UINT uMsg, DWORD dwUser, DWORD dw1, DWORD dw2);
void prepareRender(void* p_audio_data, uint8_t** pp_pcm_buffer , size_t size);
void handleStream(void* p_audio_data, uint8_t* p_pcm_buffer, unsigned int channels,
                  unsigned int rate, unsigned int nb_samples, unsigned int bits_per_sample, size_t size, int64_t pts);
void audioCleanup();
