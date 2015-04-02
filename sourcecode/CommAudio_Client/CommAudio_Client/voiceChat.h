/*------------------------------------------------------------------------------------------------------------------
-- SOURCE FILE: voiceChat.h
--
-- PROGRAM: CommAudio_Client
--
-- DATE: April 1, 2015
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

bool StartVoiceChat(in_addr dest);
bool EndVoiceChat();

DWORD WINAPI StartVoiceRecv(LPVOID parameter);
bool StartVoicePlayback();
void RecvVoiceData();
void CALLBACK VoiceOutCallback(HWAVEOUT hWave, UINT uMsg, DWORD dwUser, DWORD dw1, DWORD dw2);

DWORD WINAPI StartVoiceSend(LPVOID parameter);
DWORD WINAPI StartVoiceRecord(LPVOID parameter);
void CALLBACK VoiceInCallback(HWAVEOUT hWave, UINT uMsg, DWORD dwUser, DWORD dw1, DWORD dw2);

#endif
