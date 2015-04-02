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

#include <mmsystem.h>

/******************************************************************************************************************************************
*******************************************************************************************************************************************
*
* START AND END FUNCTIONS
*
*******************************************************************************************************************************************
******************************************************************************************************************************************/

/*-------------------------------------------------------------------------------------------------
-- FUNCTION: StartMulticast
--
-- DATE: April 1, 2015
--
-- DESIGNER: Michael Chimick
--
-- PROGRAMMER: Michael Chimick
--
-- INTERFACE: bool StartMulticast()
--
-- RETURNS: bool // returns true is all calls successful, false otherwise
--
-- NOTES:
--    Starts receiving and playback threads for the multicasting data
-------------------------------------------------------------------------------------------------*/
bool StartMulticast();

/*-------------------------------------------------------------------------------------------------
-- FUNCTION: EndMulticast
--
-- DATE: April 1, 2015
--
-- DESIGNER: Michael Chimick
--
-- PROGRAMMER: Michael Chimick
--
-- INTERFACE: bool EndMulticast()
--
-- RETURNS: bool // returns true is all calls successful, false otherwise
--
-- NOTES:
--    Stops the receiving and playback threads, and resets variables
-------------------------------------------------------------------------------------------------*/
bool EndMulticast();

/******************************************************************************************************************************************
*******************************************************************************************************************************************
*
* RECEIVE AND PLAYBACK FUNCTIONS
*
*******************************************************************************************************************************************
******************************************************************************************************************************************/

/*-------------------------------------------------------------------------------------------------
-- FUNCTION: JoinMulticast
--
-- DATE: April 1, 2015
--
-- DESIGNER: Michael Chimick
--
-- PROGRAMMER: Michael Chimick, Rhea Lauzon
--
-- INTERFACE: DWORD WINAPI JoinMulticast(LPVOID parameter)
--                LPVOID parameter // NULL pointer
--
-- RETURNS: DWORD // thread exit code
--
-- NOTES:
--    Initializes multicast variables and starts the playback thread and receive function
-------------------------------------------------------------------------------------------------*/
DWORD WINAPI JoinMulticast(LPVOID parameter);

/*-------------------------------------------------------------------------------------------------
-- FUNCTION: StartMulticastPlayback
--
-- DATE: April 1, 2015
--
-- DESIGNER: Michael Chimick, Rhea Lauzon
--
-- PROGRAMMER: Michael Chimick, Rhea Lauzon
--
-- INTERFACE: DWORD WINAPI StartMulticastPlayback(LPVOID parameter)
--                LPVOID parameter // NULL pointer
--
-- RETURNS: DWORD // thread exit code, -1 on failure; 0 on success
--
-- NOTES:
--    Starts the multicast playback
-------------------------------------------------------------------------------------------------*/
void CALLBACK MultiPlaybackCallback(HWAVEOUT hWave, UINT uMsg, DWORD dwUser, DWORD dw1, DWORD dw2);

/*-------------------------------------------------------------------------------------------------
-- FUNCTION: RecvMulticastData
--
-- DATE: April 1, 2015
--
-- DESIGNER: Michael Chimick, Rhea Lauzon
--
-- PROGRAMMER: Michael Chimick, Rhea Lauzon
--
-- INTERFACE: void RecvMulticastData()
--
-- RETURNS: void
--
-- NOTES:
--    Runs a loop that receives voice data and places it into the buffer
-------------------------------------------------------------------------------------------------*/
void RecvMulticastData();

/*-------------------------------------------------------------------------------------------------
-- FUNCTION: MultiPlaybackCallback
--
-- DATE: April 1, 2015
--
-- DESIGNER: Michael Chimick
--
-- PROGRAMMER: Michael Chimick
--
-- INTERFACE: void CALLBACK MultiPlaybackCallback(HWAVEOUT hWave, UINT uMsg, DWORD dwUser, DWORD dw1, DWORD dw2)
--                HWAVEOUT hWave // handle to the waveform-audio device associated with the callback
--                UINT uMsg      // waveform-audio output message
--                DWORD dwUser   // user-instance data
--                DWORD dw1      // message parameter; wavehdr outputted
--                DWORD dw2      // message parameter
--
-- RETURNS: void
--
-- NOTES:
--    Callback after a buffer has finished playing, simply adds it to the back of the queue
-------------------------------------------------------------------------------------------------*/
DWORD WINAPI StartMulticastPlayback(LPVOID arg);

#endif
