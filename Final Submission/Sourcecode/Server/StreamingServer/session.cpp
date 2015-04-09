/*---------------------------------------------------------------------------------------------
-- SOURCE FILE: session.cpp -
--
-- PROGRAM:
--
-- DATE: March 22, 2015
--
-- Interface:
        bool createSession(SOCKET c, char* a);
        LPMUSIC_SESSION getSession(SOCKET s);
        bool createSems(LPMUSIC_SESSION m);
        bool createThreads(LPMUSIC_SESSION m);
        bool initSockets(LPMUSIC_SESSION m, SOCKET control);

        DWORD WINAPI micSendThread(LPVOID lpParameter);
        DWORD WINAPI micRcvThread(LPVOID lpParameter);
        DWORD WINAPI sendFileThread(LPVOID lpParameter);
        DWORD WINAPI controlThread(LPVOID lpParameter);
        DWORD WINAPI AcceptThread(LPVOID lpParameter);

        void CALLBACK controlRoutine(DWORD Error, DWORD BytesTransferred, LPWSAOVERLAPPED Overlapped, DWORD InFlags);
        void CALLBACK sendFileRoutine(DWORD Error, DWORD BytesTransferred, LPWSAOVERLAPPED Overlapped, DWORD InFlags);
        bool startSend(LPMUSIC_SESSION m, string filename);
        void sessionCleanUp(LPMUSIC_SESSION m);
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Jeff Bayntun
--
-- PROGRAMMER: Jeff Bayntun
--
--------------------------------------------------------------------------------------------*/
#include <iostream>
#include "stdafx.h"
#include "session.h"
#include "helper.h"
#include <mmsystem.h>
#include <vlc/vlc.h>
#include <vlc/libvlc.h>
#include "music.h"

using namespace std;

HANDLE newSongSem;
HANDLE songAccessSem;
HANDLE userChangeSem;
HANDLE userAccessSem;
vector<string> userList;
vector<string> songList;
HANDLE sessionsSem;
string multicastSong;

map<SOCKET, LPMUSIC_SESSION> SESSIONS;


/*******************************************************************
** Function: createSession()
**
** Date: March 22th, 2015
**
** Revisions:
**
**
** Designer: Jeff Bayntun
**
** Programmer: Jeff Bayntun
**
** Interface:
**			bool createSession(SOCKET c, char* a)
**          c: control socket for this session
**          a: ip address of client
**
**
** Returns:
**			false on failure
**
** Notes: Creates the threads and semaphores specific to this session
*******************************************************************/
bool createSession(SOCKET c, char* a)
{
    cout << endl << "Creating a session for control socket " << c << " with ip " << a << endl;
    LPMUSIC_SESSION m;
    if ((m = (LPMUSIC_SESSION) GlobalAlloc(GPTR,
         sizeof(MUSIC_SESSION))) == NULL)
      {
         printf("music GlobalAlloc() failed with error %d\n", GetLastError());
         return false;
      }

    if( !( createSocketInfo( &(m->control), c ) && createThreads(m) && createSems(m) ))
    {
        return false;
    }
    // should be ok
    strcpy_s(m->ip, a);
    WaitForSingleObject(userAccessSem, INFINITE);
    userList.emplace_back(a);
    ReleaseSemaphore(userAccessSem, 1, 0);

    WaitForSingleObject(sessionsSem, INFINITE);
    SESSIONS.insert(SessionPair(c, m));
    ReleaseSemaphore(userChangeSem, SESSIONS.size(), 0);
    ReleaseSemaphore(sessionsSem, 1, 0);

    //start the threads here
    ResumeThread(m->control_thr);
    ResumeThread(m->voice_thr);
    ResumeThread(m->send_thr);

    return true;
}

/*******************************************************************
** Function: initSockets()
**
** Date: March 22th, 2015
**
** Revisions:
**
**
** Designer: Jeff Bayntun
**
** Programmer: Jeff Bayntun
**
** Interface:
**			bool initSockets(LPMUSIC_SESSION m, SOCKET control)
**          m: this music session struct
**          control: the control socket for this structure
**
**
** Returns:
**			false on failure
**
** Notes: initializes sockets for the mic and file sending
*******************************************************************/
bool initSockets(LPMUSIC_SESSION m, SOCKET control)
{
    SOCKET mic_out = createUDPSOCKET();
    SOCKET mic_in = createUDPSOCKET();
    return createSocketInfo(&(m->control), control)
            && createSocketInfo(&(m->voice), mic_in);
}
/*******************************************************************
** Function: createThreads()
**
** Date: March 22th, 2015
**
** Revisions:
**
**
** Designer: Jeff Bayntun
**
** Programmer: Jeff Bayntun
**
** Interface:
**			bool createThreads(LPMUSIC_SESSION m)
**          m: this music session struct
**
**
** Returns:
**			false on failure
**
** Notes: creates threads for this session
*******************************************************************/
bool createThreads(LPMUSIC_SESSION m)
{
    return createWorkerThread(controlThread, &(m->control_thr), m, CREATE_SUSPENDED)
        && createWorkerThread(voiceThread, &(m->voice_thr), m, CREATE_SUSPENDED)
        && createWorkerThread(sendFileThread, &(m->send_thr), m, CREATE_SUSPENDED);
}
/*******************************************************************
** Function:  AcceptThread()
**
** Date: March 22th, 2015
**
** Revisions:
**
**
** Designer: Jeff Bayntun
**
** Programmer: Jeff Bayntun
**
** Interface:
**			DWORD WINAPI AcceptThread(LPVOID lpParameter)
**          param unused
**
**
** Returns:
**			FALSE on failure
**
** Notes: thread that listens and waits for incoming connections
*******************************************************************/
DWORD WINAPI AcceptThread(LPVOID lpParameter)
{
    SOCKET Accept, temp;
    sockaddr peer;
    int peer_len;

    if( (sessionsSem = CreateSemaphore(NULL, 1, 1, NULL)) == NULL)
    {
        printf("error creating sessionSem\n");
        return FALSE;
    }
    cout << endl << "Accepting Clients" << endl;
    if(!openListenSocket(&Accept, SERVER_TCP_LISTEN_PORT) )
        return FALSE;


    cout << "Id of Accepting thread " << GetCurrentThreadId() << endl;
        while(TRUE)
       {
           peer_len = sizeof(peer);
           ZeroMemory(&peer, peer_len);

           temp = accept(Accept, &peer, &peer_len);

            getpeername(temp, &peer, &peer_len);
            sockaddr_in* s_in = (struct sockaddr_in*)&peer;
            char* temp_addr = inet_ntoa( s_in->sin_addr );

            printf("Socket %d is connected to ", temp);
            printIP(*s_in);

            createSession(temp, temp_addr);
       }

    return TRUE;
}

/*******************************************************************
** Function:  controlThread()
**
** Date: March 22th, 2015
**
** Revisions:
**      March 30th, 2015
**      Julian Brandrick
**          -> Changed line 239 for VS compiler:
**                  HANDLE waitHandles[handles]; -> HANDLE waitHandles[3];
**
**
** Designer: Jeff Bayntun
**
** Programmer: Jeff Bayntun
**
** Interface:
**			DWORD WINAPI controlThread(LPVOID lpParameter)
**
**
** Returns:
**			FALSE on failure
**
** Notes: each session has its own control thread.  This is used
**  to coordinate other threads and activities
*******************************************************************/
DWORD WINAPI controlThread(LPVOID lpParameter)
{
    DWORD RecvBytes, result, flags, handles;
    handles = 3;
    HANDLE waitHandles[3];

    flags = 0;
    LPMUSIC_SESSION m = (LPMUSIC_SESSION) lpParameter;

    waitHandles[0] = userChangeSem;
    waitHandles[1] = newSongSem;
    waitHandles[2] = m->sendCompleteSem;

    // send song list
    sendSongList(m);
    
    
    //post rcv call waiting for data
    if ( WSARecv( m->control.Socket, &(m->control.DataBuf), 1, &RecvBytes, &flags, &(m->control.Overlapped), controlRoutine ) == SOCKET_ERROR)
      {
         if (WSAGetLastError() != WSA_IO_PENDING)
         {
            printf("WSARecv() failed with error %d\n", WSAGetLastError());
            sessionCleanUp(m);
         }
      } 

    // get into alertable state
    while(1)
    {
        result = WaitForMultipleObjectsEx(handles, waitHandles, FALSE, INFINITE, TRUE);
        if(result == WAIT_FAILED)
        {
            perror("error waiting for multiple objects");
            sessionCleanUp(m);
        }

        if (result == WAIT_IO_COMPLETION)
         {
            continue;
         }

         switch(result -  WAIT_OBJECT_0)
         {
            case 0:
                 // change in user list, access the list and send it
                updateNewUser(m);
                 break;
            case 1:
                 //new multicast song, send name of new song
                 break;
            case 2:
                 //finished sending unicast to this client
                 break;
         default:
                //error of somekind, clean up sessions and exit
                sessionCleanUp(m);
         }
    }

    return FALSE;
}
/*******************************************************************
** Function:  controlRoutine()
**
** Date: March 22th, 2015
**
** Revisions:
**
**
** Designer: Jeff Bayntun
**
** Programmer: Jeff Bayntun
**
** Interface:
**			void CALLBACK controlRoutine(DWORD Error, DWORD BytesTransferred, LPWSAOVERLAPPED Overlapped, DWORD InFlags)
**          Error: error flags
**          BytesTransferred: the number of bytes transfered in or out by this call
**          Overlapped: the overlapped object for this socket
**          InFlags: flags passed in
**
** Returns:
**			void
**
** Notes: callback function for control thread completion routine.
** responds to data received or send as appropriate.
*******************************************************************/
void CALLBACK controlRoutine(DWORD Error, DWORD BytesTransferred, LPWSAOVERLAPPED Overlapped, DWORD InFlags)
{
    //do like control in other one, if send make sure all is sent, if rcv, do based on thing recieved action
    // search set of sessions using the control LPSOCKET

    DWORD SendBytes, RecvBytes;
   DWORD Flags;
   string message_rcv;
   LPMUSIC_SESSION m;
   ctrlMessage ctrl;


   // Reference the WSAOVERLAPPED structure as a SOCKET_INFORMATION structure
   LPSOCKET_INFORMATION SI = (LPSOCKET_INFORMATION) Overlapped;
   m = getSession(SI->Socket); // null check...

   if (Error != 0)
   {
     printf("I/O operation failed with error %d\n", Error);
   }

   if (BytesTransferred == 0) // nothing sent or rcvd...
   {
      printf("Closing control socket %d\n", SI->Socket);
   }

   if (Error != 0 || BytesTransferred == 0)
   {
      sessionCleanUp(m);
   }

   // Check to see if the BytesRECV field equals zero. If this is so, then
   // this means a WSARecv call just completed so update the BytesRECV field
   // with the BytesTransferred value from the completed WSARecv() call.

   if (SI->BytesRECV == 0) // after a rcv
   {
      SI->BytesRECV = BytesTransferred;
      SI->BytesSEND = 0;
      //rcv = true;
      message_rcv.assign(SI->Buffer);
      parseControlString(message_rcv, &ctrl);
      switch(ctrl.type)
      {
          //someone wants to make a mic chat with this client
          case MIC_CONNECTION:
          {
              m->mode = 'u';
              strcpy_s(m->ptp_ip, ctrl.msgData[0].c_str());
              ReleaseSemaphore(m->voiceSem, 1, 0);
              break;
          }
          //song has been requested for unicast send to this client
          case SONG_REQUEST:
          {
              m->mode = 'u';
              startSend(m, ctrl.msgData[0]);
              break;
          }

          //song has been requested for tcp send to this client
          case SAVE_SONG:
          {
            m->mode = 't';
           // startSend(m, ctrl.msgData[0]);
              break;
          }
          case END_CONNECTION:
          default:
              {

              }
      }

       SI->bytesToSend = BytesTransferred; // send back same size, that is what they're expecting
   }
   else // after a send
   {
      SI->BytesSEND += BytesTransferred;
      SI->bytesToSend -= BytesTransferred;
   }

   if (SI->bytesToSend > 0) // send or continue sending
   {

      // Post another WSASend() request.
      // Since WSASend() is not gauranteed to send all of the bytes requested,
      // continue posting WSASend() calls until all received bytes are sent.

      ZeroMemory(&(SI->Overlapped), sizeof(WSAOVERLAPPED));

      SI->DataBuf.buf = SI->Buffer + SI->BytesSEND;
      SI->DataBuf.len = SI->bytesToSend;

      if (WSASend(SI->Socket, &(SI->DataBuf), 1, &SendBytes, 0,
         &(SI->Overlapped), controlRoutine) == SOCKET_ERROR)
      {
         if (WSAGetLastError() != WSA_IO_PENDING)
         {
            printf("WSASend() failed with error %d\n", WSAGetLastError());
            return;
         }
      }
   }
   else // all was sent, ready for next RECV
   {
      SI->BytesRECV = 0;
      SI->bytesToSend = 0; // just in case it got negative somehow.....
      delete [] &(SI->DataBuf);

      // Now that there are no more bytes to send post another WSARecv() request.

      Flags = 0;
      ZeroMemory(&(SI->Overlapped), sizeof(WSAOVERLAPPED));
      ZeroMemory(&(SI->Buffer), sizeof(DATA_BUFSIZE));

      SI->DataBuf.len = DATA_BUFSIZE;
      SI->DataBuf.buf = SI->Buffer; // should this be zeroed??

      if (WSARecv(SI->Socket, &(SI->DataBuf), 1, &RecvBytes, &Flags,
         &(SI->Overlapped), controlRoutine) == SOCKET_ERROR)
      {
         if (WSAGetLastError() != WSA_IO_PENDING )
         {
            printf("WSARecv() failed with error %d\n", WSAGetLastError());
            return;
         }
      }
   }
}

/*******************************************************************
** Function:  sendFileRoutine()
**
** Date: March 22th, 2015
**
** Revisions:
**
**
** Designer: Jeff Bayntun
**
** Programmer: Jeff Bayntun
**
** Interface:
**			void CALLBACK sendFileRoutine(DWORD Error, DWORD BytesTransferred, LPWSAOVERLAPPED Overlapped, DWORD InFlags)
**          Error: error flags
**          BytesTransferred: the number of bytes transfered in or out by this call
**          Overlapped: the overlapped object for this socket
**          InFlags: flags passed in
**
**
** Returns:
**			void
**
** Notes: callback function for sendFile thread completion routine.
** calls send until whole file has been sent.  Works for both
** TCP and UDP
*******************************************************************/
void CALLBACK sendFileRoutine(DWORD Error, DWORD BytesTransferred, LPWSAOVERLAPPED Overlapped, DWORD InFlags)
{
    // adjust data pointers
    // close socket when all data has been sent
    // need to alert control when we close the socket


    DWORD SendBytes;
    LPMUSIC_SESSION m;
    sockaddr_in client;

   // Reference the WSAOVERLAPPED structure as a SOCKET_INFORMATION structure
   LPSOCKET_INFORMATION SI = (LPSOCKET_INFORMATION) Overlapped;
   m = getSession(SI->Socket);

   if (Error != 0)
   {
     printf("I/O operation failed with error %d\n", Error);
   }

   if (BytesTransferred == 0) // nothing sent or rcvd...
   {
      printf("Closing send socket %d\n", SI->Socket);
   }

   if (Error != 0 || BytesTransferred == 0)
   {
      deleteSocketInfo(SI);
      m->sending = false;
      return;
   }

      SI->BytesSEND += BytesTransferred;
      SI->bytesToSend -= BytesTransferred;

   if (SI->bytesToSend > 0) // send or continue sending
   {
       int port = (m->mode == 't') ? CLIENT_TCP_PORT: CLIENT_UDP_PORT;
        getIP_Addr(&client, m->ip, port);

      // Post another WSASend() request.
      // Since WSASend() is not gauranteed to send all of the bytes requested,
      // continue posting WSASend() calls until all received bytes are sent.

      ZeroMemory(&(SI->Overlapped), sizeof(WSAOVERLAPPED));

      SI->DataBuf.buf = SI->Buffer + SI->BytesSEND;
      SI->DataBuf.len = SI->bytesToSend;

      if (WSASendTo(m->send.Socket, &(m->send.DataBuf), 1, &SendBytes, 0, (struct sockaddr *)&client, sizeof(client), &(m->send.Overlapped), sendFileRoutine) == SOCKET_ERROR)
        {
            if (WSAGetLastError() != WSA_IO_PENDING)
            {
            printf("WSASend() failed with error %d\n", WSAGetLastError());
            m->sending = false;
            return;
            }
        }
   }
   else // all was sent, close this down
   {
     m->sending = false;

   }

}

/*******************************************************************
** Function:  sendFileThread()
**
** Date: March 22th, 2015
**
** Revisions:
**
**
** Designer: Jeff Bayntun
**
** Programmer: Jeff Bayntun
**
** Interface:
**			DWORD WINAPI sendFileThread(LPVOID lpParameter)
**          lpParameter is a pointer to the session object this send is for
**
**
** Returns:
**			void
**
** Notes: starting point for sendFile Thread. Waits for signal from
** control thread, then loads the file and makes the initial
** WSA send call.
*******************************************************************/
DWORD WINAPI sendFileThread(LPVOID lpParameter)
{
    LPMUSIC_SESSION m = (LPMUSIC_SESSION) lpParameter;
    DWORD result, SendBytes;
    sockaddr_in client;
    SOCKET temp;
    int port;

    while(1)
    {

        //wait for signal from control to initiate a send
        WaitForSingleObject(m->sendSem, INFINITE);
        // should check for error

        m->sending = true;
        port = (m->mode == 't') ? CLIENT_TCP_PORT: CLIENT_UDP_PORT;
        getIP_Addr(&client, m->ip, port);

        if(m->mode == 't')
        {
            temp = createTCPSOCKET();
            createSocketInfo(&(m->send), temp);
            if( !connectTCPSOCKET(m->send.Socket, &client) )
            {
                printf("error on tcp send connecting for session %d", m->control.Socket);
                    return FALSE;
            }
        }
        else
        {
            temp = createUDPSOCKET();
            createSocketInfo(&(m->send), temp);
        }

        unsigned long size = loadFile(m->filename, &(m->fileToSend));
        if(size == -1)
        {
            m->sending = false;
            deleteSocketInfo(&(m->send));
            continue;
        }

        m->send.bytesToSend = size;
        m->send.BytesSEND = 0;
        m->send.DataBuf.buf = m->fileToSend;
        m->send.DataBuf.len = m->send.bytesToSend;

        //call send function
        if (WSASendTo(m->send.Socket, &(m->send.DataBuf), 1, &SendBytes, 0, (struct sockaddr *)&client, sizeof(client), &(m->send.Overlapped), sendFileRoutine) == SOCKET_ERROR)
        {
            if (WSAGetLastError() != WSA_IO_PENDING)
            {
            printf("WSASend() failed with error %d\n", WSAGetLastError());
            delete [] m->fileToSend;
            return FALSE;
            }
        }

        //get into waiting state
        while(1)
        {
            result = SleepEx(INFINITE, TRUE);

             if (result!= WAIT_IO_COMPLETION)
             {
                printf("error on tcp send for session %d", m->control.Socket);
                delete [] m->fileToSend;
                break;
             }

             if( !m->sending )
             {
                 deleteSocketInfo(&(m->send));
                 delete [] m->fileToSend;
                 ReleaseSemaphore(m->sendCompleteSem, 1, NULL);
             }
        }
    }

    return TRUE;
}
/*******************************************************************
** Function:  getSession()
**
** Date: March 22th, 2015
**
** Revisions:
**
**
** Designer: Jeff Bayntun
**
** Programmer: Jeff Bayntun
**
** Interface:
**			LPMUSIC_SESSION getSession(SOCKET s)
**          s: socket to find a session for
**
**
** Returns:
**			void
**
** Notes: returns a session struct that the given socket belongs to.
** Looks in the SESSIONS map.
**  return nullptr on failure
*******************************************************************/
LPMUSIC_SESSION getSession(SOCKET s)
{
    LPMUSIC_SESSION m;
    WaitForSingleObject(sessionsSem, INFINITE);
    for(auto const &it : SESSIONS)
    {
        // it.first is the first key
        // it.second is the data
        // was &it.second before... i think i would need a ** to music session for that though...
        m = (LPMUSIC_SESSION)it.second;
        if( m->control.Socket == s || m->voice.Socket == s || m->send.Socket == s)
        {
            ReleaseSemaphore(sessionsSem, 1, 0);
            return m;
        }
    }
    ReleaseSemaphore(sessionsSem, 1, 0);
    return nullptr;
}
/*******************************************************************
** Function:  startSend()
**
** Date: March 22th, 2015
**
** Revisions:
**
**
** Designer: Jeff Bayntun
**
** Programmer: Jeff Bayntun
**
** Interface:
**			bool startSend(LPMUSIC_SESSION m, string filename)
**          m: this session struct
**          filename: name of file to send
**
**
** Returns:
**			void
**
** Notes: control thread signals to sendFile thread to start sending
** this file
*******************************************************************/
bool startSend(LPMUSIC_SESSION m, string filename)
{
    //verify file is on the list, if not return false
    //add it to the session
     strcpy_s( m->filename, filename.c_str() );
     ReleaseSemaphore(m->sendSem, 1, 0);

     return true;
}
/*******************************************************************
** Function:  createSems()
**
** Date: March 22th, 2015
**
** Revisions:
**
**
** Designer: Jeff Bayntun
**
** Programmer: Jeff Bayntun
**
** Interface:
**			bool createSems(LPMUSIC_SESSION m)
**          m: session struct to create semaphores for
**
**
** Returns:
**		 true on success
**
** Notes: creates the semaphores used by this session
*******************************************************************/
bool createSems(LPMUSIC_SESSION m)
{
    if( (m->sendSem = CreateSemaphore(NULL, 0, 1, NULL)) == NULL )
    {
        printf("error creating semaphores");
        return false;
    }

    if( (m->sendCompleteSem = CreateSemaphore(NULL, 0, 1, NULL)) == NULL )
    {
        printf("error creating semaphores");
        return false;
    }

    if( (m->voiceSem = CreateSemaphore(NULL, 0, 1, NULL)) == NULL )
    {
        printf("error creating semaphores");
        return false;
    }

    return true;
}
/*******************************************************************
** Function:  sessionCleanUp()
**
** Date: March 22th, 2015
**
** Revisions:
**
**
** Designer: Jeff Bayntun
**
** Programmer: Jeff Bayntun
**
** Interface:
**			void sessionCleanUp(LPMUSIC_SESSION m)
**          m: session struct to create semaphores for
**
**
** Returns:
**		 void
**
** Notes: closes threads, SOCKET_INFORMATIONS, and semaphores of
** the input session.
*******************************************************************/
void sessionCleanUp(LPMUSIC_SESSION m)
{
    SOCKET c = m->control.Socket;
    printf("Session clean up for socket %d\n", c);

    //remove this user from the list
    WaitForSingleObject(userAccessSem, INFINITE);
    auto it = userList.begin();
    while(it != userList.end())
    {
        if(strcmp( (*it).c_str(), m->ip) == 0 )
        {
            userList.erase(it);
            break;
        }
        it++;
    }
    ReleaseSemaphore(userAccessSem, 1, 0);

    // close threads
    TerminateThread(m->voice_thr, 0);
    TerminateThread(m->send_thr, 0);
    TerminateThread(m->voice_thr, 0);


    //delete any data in the buffers
    //delete [] &(m->control.DataBuf);
   // delete [] &(m->send.DataBuf);

    // close all socket infos
    if(&(m->control) == 0 )
        deleteSocketInfo(&(m->control));
    if(&(m->voice) == 0 )
        deleteSocketInfo(&(m->voice));
    if(&(m->send) == 0 )
        deleteSocketInfo(&(m->send));

    // close semaphores
    CloseHandle(m->voiceSem);
    CloseHandle(m->sendSem);
    CloseHandle(m->sendCompleteSem);

    // close thread handles
    CloseHandle(m->voice_thr);
    CloseHandle(m->send_thr);

    //delete session from map
    WaitForSingleObject(sessionsSem, INFINITE);
    SESSIONS.erase(c);
    ReleaseSemaphore(sessionsSem, 1, NULL);

    //signal other sessions to send the updated userlist
    ReleaseSemaphore(userChangeSem, SESSIONS.size(), 0);

    //close this thread
    ExitThread(0);
}

DWORD WINAPI voiceThread(LPVOID lpParameter)
{
    LPMUSIC_SESSION m = (LPMUSIC_SESSION) lpParameter;
    DWORD index;
	DWORD flags = 0;
	DWORD recvBytes = 0;
	SOCKADDR_IN client;
    int clientLen = sizeof(client);

	do
	{
        //wait for signal from control to initiate a send
        WaitForSingleObject(m->voiceSem, INFINITE);

        m->sending = false;

        if(WSARecvFrom(m->voice.Socket, &(m->voice.DataBuf), 1, &recvBytes, &flags, (sockaddr *)&(client), &clientLen, &(m->voice.Overlapped), voiceRoutine) == SOCKET_ERROR)
		{
			if (WSAGetLastError() != WSA_IO_PENDING)
		    {
		        displayError("WSARecvFrom failed", WSAGetLastError()); 
		        return -1;
		    }
		}
		
		while(TRUE)
		{
			index = SleepEx(INFINITE, TRUE);
		
			if(index == WAIT_IO_COMPLETION)
			{
				break;
			}
			else
			{
				displayError("SleepEx failed", index);
				return -1;
			}
		}
	}
	while(recvBytes > 0);

    ReleaseSemaphore(m->voiceSem, 1, 0);

    return TRUE;
}

void CALLBACK voiceRoutine(DWORD error, DWORD bytesTransferred, LPWSAOVERLAPPED lpOverlapped, DWORD dwFlags)
{
	DWORD flags = 0;
	DWORD recvBytes;
	DWORD sendBytes;
	LPSOCKET_INFORMATION SI = (LPSOCKET_INFORMATION)lpOverlapped;
    LPMUSIC_SESSION m = getSession(SI->Socket);
    SOCKADDR_IN clientFrom;
    SOCKADDR_IN clientTo;
	int clientFromLen;
	int clientToLen;

    int port = CLIENT_UDP_PORT;
    getIP_Addr(&clientFrom, m->ip, port);
    clientFromLen = sizeof(clientFrom);
    getIP_Addr(&clientTo, m->ptp_ip, port);
    clientToLen = sizeof(clientTo);

    if(error != 0)
    {
        displayError("Voice error", error);
    }
    if(bytesTransferred == 0)
    {
        printf("Closing send socket %d\n", SI->Socket);
    }

	if(error != 0 || bytesTransferred == 0)
	{
        deleteSocketInfo(SI);
        m->sending = false;
		return;
	}

    if(SI->BytesRECV == 0)
    {
        SI->BytesRECV = bytesTransferred;
        SI->BytesSEND = 0;
    }
    else
    {
        SI->BytesSEND += bytesTransferred;
    }

    if(SI->BytesRECV > SI->BytesSEND)
    {
        ZeroMemory(&SI->Overlapped, sizeof(WSAOVERLAPPED));

        SI->DataBuf.buf = SI->Buffer + SI->BytesSEND;
        SI->DataBuf.len = SI->BytesRECV - SI->BytesSEND;

	    if(WSASendTo(SI->Socket, &(SI->DataBuf), 1, &sendBytes, flags, (sockaddr *)&(clientTo), clientToLen, &(SI->Overlapped), voiceRoutine) == SOCKET_ERROR)
	    {
	    	if (WSAGetLastError() != WSA_IO_PENDING)
            {
                displayError("WSASendTo failed", WSAGetLastError()); 
                m->sending = false;
                return;
            }
	    }
    }
    else
    {
        SI->BytesRECV = 0;
        flags = 0;
        m->sending = false;

        ZeroMemory(&SI->Overlapped, sizeof(WSAOVERLAPPED));

        SI->DataBuf.len = DATA_BUFSIZE;
        SI->DataBuf.buf = SI->Buffer;

        if(WSARecvFrom(SI->Socket, &(SI->DataBuf), 1, &recvBytes, &flags, (sockaddr *)&(clientFrom), &clientFromLen, &(SI->Overlapped), voiceRoutine) == SOCKET_ERROR)
	    {
	    	if (WSAGetLastError() != WSA_IO_PENDING)
            {
                displayError("WSARecvFrom failed", WSAGetLastError()); 
                return;
            }
	    }
    }
}


void sendSongList(LPMUSIC_SESSION m)
{
    if(songList.empty())
        return;

    DWORD result, SendBytes;
    sockaddr_in client;
    string temp;
    ctrlMessage message;

    message.msgData = songList;
    message.type = LIBRARY_INFO;
    createControlString(message, temp);

    string to_send = "********************************************" + temp;

    sendTCPMessage(&(m->control.Socket), to_send, DATA_BUFSIZE);

}

void sendUserList(LPMUSIC_SESSION m)
{
    if(userList.empty())
        return;

    string temp;

    ctrlMessage message;
    message.msgData = userList;
    message.type = CURRENT_LISTENERS;
    createControlString(message, temp);

    string to_send = "********************************************" + temp;

    //call send function
    sendTCPMessage(&(m->control.Socket), to_send, DATA_BUFSIZE);
}

void updateNewUser(LPMUSIC_SESSION m)
{
	sendUserList(m);
	ctrlMessage message;
	string temp;

	MetaData *d = new MetaData;
	fetchMetaData(d);
	//create the now playing control message
	stringstream ss;
	ss << d->title << "^" << d->artist << "^" << d->album << "^";
	message.msgData.push_back(ss.str());
    message.type = NOW_PLAYING;

    createControlString(message, temp);

    string to_send = "********************************************" + temp;

	//send the message to the client
	sendTCPMessage(&(m->control.Socket), to_send, DATA_BUFSIZE);

	delete d;

}

void sendToAll(string message)
{
    vector<SOCKET> sockets;
    WaitForSingleObject(sessionsSem, INFINITE);
    for(auto const &it : SESSIONS)
    {
         sockets.push_back((SOCKET)it.first);
    }
    ReleaseSemaphore(sessionsSem, 1, 0);

    for(auto s: sockets)
    {
        sendTCPMessage(&(s), message, DATA_BUFSIZE);
    }
}

void sendNowPlaying(string artist, string name, string album, string length)
{
	string temp;
    ctrlMessage message;

	//create the control message
	stringstream ss;
	ss << name << "^" << artist << "^" << album << "^";
	message.msgData.push_back(ss.str());
    message.type = NOW_PLAYING;

    createControlString(message, temp);

    string to_send = "********************************************" + temp;

    //call send function
	sendToAll(to_send);
}