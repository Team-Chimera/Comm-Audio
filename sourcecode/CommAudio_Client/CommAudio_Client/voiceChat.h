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

/******************************************************************************************************************************************
*******************************************************************************************************************************************
*
* START AND END FUNCTIONS
*
*******************************************************************************************************************************************
******************************************************************************************************************************************/

/*-------------------------------------------------------------------------------------------------
-- FUNCTION: StartVoiceChat
--
-- DATE: April 1, 2015
--
-- DESIGNER: Michael Chimick
--
-- PROGRAMMER: Michael Chimick
--
-- INTERFACE: bool StartVoiceChat(hostent * dest)
--                hostent * dest // valid ip for the server
--
-- RETURNS: bool // returns true is all calls successful, false otherwise
--
-- NOTES:
--    Starts receiving, playback, recording, and sending of voice chat data
-------------------------------------------------------------------------------------------------*/
bool StartVoiceChat(in_addr dest);

/*-------------------------------------------------------------------------------------------------
-- FUNCTION: EndVoiceChat
--
-- DATE: April 1, 2015
--
-- DESIGNER: Michael Chimick
--
-- PROGRAMMER: Michael Chimick
--
-- INTERFACE: bool EndVoiceChat()
--
-- RETURNS: bool // returns true is all calls successful, false otherwise
--
-- NOTES:
--    Terminates the voice chat threads, and resets necessary variables
-------------------------------------------------------------------------------------------------*/
bool EndVoiceChat();

/******************************************************************************************************************************************
*******************************************************************************************************************************************
*
* RECEIVE AND PLAY FUNCTIONS
*
*******************************************************************************************************************************************
******************************************************************************************************************************************/

/*-------------------------------------------------------------------------------------------------
-- FUNCTION: StartVoiceRecv
--
-- DATE: April 1, 2015
--
-- DESIGNER: Michael Chimick
--
-- PROGRAMMER: Michael Chimick
--
-- INTERFACE: DWORD WINAPI StartVoiceRecv(LPVOID parameter)
--                LPVOID parameter // NULL pointer
--
-- RETURNS: DWORD // thread exit code
--
-- NOTES:
--    Initializes variables and starts the receive and playback functions
-------------------------------------------------------------------------------------------------*/
DWORD WINAPI StartVoiceRecv(LPVOID parameter);

/*-------------------------------------------------------------------------------------------------
-- FUNCTION: StartVoicePlayback
--
-- DATE: April 1, 2015
--
-- DESIGNER: Michael Chimick
--
-- PROGRAMMER: Michael Chimick
--
-- INTERFACE: DWORD WINAPI StartVoicePlayback(LPVOID parameter)
--                LPVOID parameter // NULL pointer
--
-- RETURNS: DWORD // thread exit code
--
-- NOTES:
--    Starts the voice playback
-------------------------------------------------------------------------------------------------*/
bool StartVoicePlayback();

/*-------------------------------------------------------------------------------------------------
-- FUNCTION: RecvVoiceData
--
-- DATE: April 1, 2015
--
-- DESIGNER: Michael Chimick
--
-- PROGRAMMER: Michael Chimick
--
-- INTERFACE: void RecvVoiceData()
--
-- RETURNS: void
--
-- NOTES:
--    Runs a loop that receives voice data and places it into the buffer
-------------------------------------------------------------------------------------------------*/
void RecvVoiceData();

/*-------------------------------------------------------------------------------------------------
-- FUNCTION: VoicePlaybackCallback
--
-- DATE: April 1, 2015
--
-- DESIGNER: Michael Chimick
--
-- PROGRAMMER: Michael Chimick
--
-- INTERFACE: void CALLBACK VoicePlaybackCallback(HWAVEOUT hWave, UINT uMsg, DWORD dwUser, DWORD dw1, DWORD dw2)
--                HWAVEOUT hWave // handle to the waveform-audio device associated with the callback
--                UINT uMsg      // waveform-audio output message
--                DWORD dwUser   // user-instance data
--                DWORD dw1      // message parameter
--                DWORD dw2      // message parameter
--
-- RETURNS: void
--
-- NOTES:
--    Callback after a buffer has finished playing, simply adds it to the back of the queue
-------------------------------------------------------------------------------------------------*/
void CALLBACK VoicePlaybackCallback(HWAVEOUT hWave, UINT uMsg, DWORD dwUser, DWORD dw1, DWORD dw2);

/******************************************************************************************************************************************
*******************************************************************************************************************************************
*
* RECORD AND SEND FUNCTIONS
*
*******************************************************************************************************************************************
******************************************************************************************************************************************/

/*-------------------------------------------------------------------------------------------------
-- FUNCTION: StartVoiceSend
--
-- DATE: April 1, 2015
--
-- DESIGNER: Michael Chimick
--
-- PROGRAMMER: Michael Chimick
--
-- INTERFACE: DWORD WINAPI StartVoiceSend(LPVOID parameter)
--                LPVOID parameter // NULL pointer
--
-- RETURNS: void
--
-- NOTES:
--    Initializes variables and starts the record and send functions
-------------------------------------------------------------------------------------------------*/
DWORD WINAPI StartVoiceSend(LPVOID parameter);

/*-------------------------------------------------------------------------------------------------
-- FUNCTION: StartVoiceRecord
--
-- DATE: April 1, 2015
--
-- DESIGNER: Michael Chimick
--
-- PROGRAMMER: Michael Chimick
--
-- INTERFACE: DWORD WINAPI StartVoiceRecord(LPVOID parameter)
--                LPVOID parameter // NULL pointer
--
-- RETURNS: void
--
-- NOTES:
--    Starts the voice recording
-------------------------------------------------------------------------------------------------*/
DWORD WINAPI StartVoiceRecord(LPVOID parameter);

/*-------------------------------------------------------------------------------------------------
-- FUNCTION: VoiceRecordCallback
--
-- DATE: April 1, 2015
--
-- REVISIONS: Created March 10, 2015
--
-- DESIGNER: Michael Chimick
--
-- PROGRAMMER: Michael Chimick
--
-- INTERFACE: void CALLBACK VoiceRecordCallback(HWAVEOUT hWave, UINT uMsg, DWORD dwUser, DWORD dw1, DWORD dw2)
--                HWAVEOUT hWave // handle to the waveform-audio device associated with the callback
--                UINT uMsg      // waveform-audio output message
--                DWORD dwUser   // user-instance data
--                DWORD dw1      // message parameter
--                DWORD dw2      // message parameter
--
-- RETURNS:
--
-- NOTES:
--    Callback after a buffer has been filled,
--    sends it to the server and then adds it back to the queue
-------------------------------------------------------------------------------------------------*/
void CALLBACK VoiceRecordCallback(HWAVEOUT hWave, UINT uMsg, DWORD dwUser, DWORD dw1, DWORD dw2);

#endif
