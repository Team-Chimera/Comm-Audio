/*------------------------------------------------------------------------------------------------------------------
-- SOURCE FILE: voiceChat.h
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

#ifndef VOICECHAT_H
#define VOICECHAT_H

#include "client.h"

bool StartVoiceChat(in_addr dest);
bool EndVoiceChat();

DWORD WINAPI StartRecvVoice(LPVOID parameter);
bool StartWaveOut();

DWORD WINAPI StartSendVoice(LPVOID parameter);
bool StartWaveIn();

DWORD WINAPI RecvVoiceThread(LPVOID parameter);
void CALLBACK RecvVoice(DWORD error, DWORD bytesTransferred, LPWSAOVERLAPPED overlapped, DWORD flags);
void CALLBACK VoiceOutCallback(HWAVEOUT hWave, UINT uMsg, DWORD dwUser, DWORD dw1, DWORD dw2);

DWORD WINAPI SendVoiceThread(LPVOID parameter);
void CALLBACK SendVoice(DWORD error, DWORD bytesTransferred, LPWSAOVERLAPPED overlapped, DWORD flags);
void CALLBACK VoiceInCallback(HWAVEOUT hWave, UINT uMsg, DWORD dwUser, DWORD dw1, DWORD dw2);

#endif
