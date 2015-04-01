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
-- This header should only be included by the voice chat files, no one else should need anything in here
--
----------------------------------------------------------------------------------------------------------------------*/

#ifndef CONTROL_VOICECHAT_H
#define CONTROL_VOICECHAT_H

#include "client.h"
#include "voiceChat.h"
#include <iostream>

DWORD WINAPI StartRecvVoice(LPVOID parameter);
bool StartVoiceOut();

DWORD WINAPI StartSendVoice(LPVOID parameter);
bool StartVoiceIn();

DWORD WINAPI RecvVoiceThread(LPVOID parameter);
void CALLBACK RecvVoice(DWORD error, DWORD bytesTransferred, LPWSAOVERLAPPED overlapped, DWORD flags);
void CALLBACK VoiceOutCallback(HWAVEOUT hWave, UINT uMsg, DWORD dwUser, DWORD dw1, DWORD dw2);

DWORD WINAPI SendVoiceThread(LPVOID parameter);
void CALLBACK SendVoice(DWORD error, DWORD bytesTransferred, LPWSAOVERLAPPED overlapped, DWORD flags);
void CALLBACK VoiceInCallback(HWAVEOUT hWave, UINT uMsg, DWORD dwUser, DWORD dw1, DWORD dw2);

#endif
