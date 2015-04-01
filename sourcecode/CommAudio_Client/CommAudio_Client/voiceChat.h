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
-- This file is what the control logic should use to access the voice chat functionality
--
----------------------------------------------------------------------------------------------------------------------*/

#ifndef VOICECHAT_H
#define VOICECHAT_H

#include <mmsystem.h>

DWORD WINAPI StartRecvVoice(LPVOID parameter);
bool StartVoiceOut();

DWORD WINAPI StartSendVoice(LPVOID parameter);
bool StartVoiceIn();

void RecvVoiceData();
void CALLBACK VoiceOutCallback(HWAVEOUT hWave, UINT uMsg, DWORD dwUser, DWORD dw1, DWORD dw2);

DWORD WINAPI SendVoiceThread(LPVOID parameter);
void CALLBACK SendVoice(DWORD error, DWORD bytesTransferred, LPWSAOVERLAPPED overlapped, DWORD flags);
void CALLBACK VoiceInCallback(HWAVEOUT hWave, UINT uMsg, DWORD dwUser, DWORD dw1, DWORD dw2);

bool StartVoiceChat(in_addr dest);
bool EndVoiceChat();

#endif
