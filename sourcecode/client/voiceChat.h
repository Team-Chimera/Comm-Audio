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

bool StartVoiceChat(in_addr dest);
bool EndVoiceChat();

#endif
