/*------------------------------------------------------------------------------------------------------------------
-- SOURCE FILE: multicast.h
--
-- PROGRAM: CommAudio_Client
--
-- DATE: March 12, 2015
--
-- REVISIONS: Created March 10, 2015
--
-- DESIGNER: Michael Chimick
--
-- PROGRAMMER: Michael Chimick
--
-- NOTES:
--
--
----------------------------------------------------------------------------------------------------------------------*/

#ifndef MULTICAST_H
#define MULTICAST_H

#include "music.h"

 #include <mmsystem.h>
bool StartMulticast(in_addr group);
bool EndMulticast();
DWORD WINAPI JoinMulticast(LPVOID parameter);

void CALLBACK MultiWaveCallback(HWAVEOUT hWave, UINT uMsg, DWORD dwUser, DWORD dw1, DWORD dw2);
void receiveMulticastData();
DWORD WINAPI playMulticastSong(LPVOID arg);
void updateVolume(int);
CircularBuffer * getCircularBuffer();
void closeAudio();



#endif
