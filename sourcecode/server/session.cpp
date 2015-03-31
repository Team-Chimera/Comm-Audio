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
        bool startSend(LPMUSIC_SESSION m, std::string filename);
        void sessionCleanUp(LPMUSIC_SESSION m);
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Jeff Bayntun
--
-- PROGRAMMER: Jeff Bayntun
--
--------------------------------------------------------------------------------------------*/

#include "session.h"


std::map<SOCKET, LPMUSIC_SESSION> SESSIONS;

using namespace std;
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

    WaitForSingleObject(sessionsSem, INFINITE);
    SESSIONS.insert(SessionPair(c, m));
    ReleaseSemaphore(sessionsSem, 1, 0);

    //start the threads here
    ResumeThread(m->control_thr);
    ResumeThread(m->mic_rcv_thr);
    ResumeThread(m->mic_send_thr);
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
            && createSocketInfo(&(m->mic_rcv), mic_in)
            && createSocketInfo(&(m->mic_send), mic_out);
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
        && createWorkerThread(micSendThread, &(m->mic_send_thr), m, CREATE_SUSPENDED)
        && createWorkerThread(micRcvThread, &(m->mic_rcv_thr), m, CREATE_SUSPENDED)
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

    if(!openListenSocket(&Accept, SERVER_TCP_LISTEN_PORT) )
        return FALSE;


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
    HANDLE waitHandles[4];

    flags = 0;
    LPMUSIC_SESSION m = (LPMUSIC_SESSION) lpParameter;

    waitHandles[0] = userChangeSem;
    waitHandles[1] = newSongSem;
    waitHandles[2] = m->sendCompleteSem;
    waitHandles[4] = updatedSongListSem;

    // post send call with song list

    // post send call with other users


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
                 break;
            case 1:
                 //new multicast song, send name of new song
                 break;
            case 2:
                 //finished sending unicast to this client
                 break;
            case 4:
                 // send updates songlist to users
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
            startSend(m, ctrl.msgData[0]);
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

    int port = (m->mode == 't') ? CLIENT_TCP_PORT: CLIENT_UDP_PORT;
    getIP_Addr(&client, m->ip, port);
    printIP(client);

    while(1)
    {

        //wait for signal from control to initiate a send
        WaitForSingleObject(m->sendSem, INFINITE);
        // should check for error

        m->sending = true;

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
        if( m->control.Socket == s || m->mic_rcv.Socket == s || m->mic_send.Socket == s || m->send.Socket == s)
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
    printf("closing thread for control socket %d\n", c);

    // close all socket infos
    deleteSocketInfo(&(m->control));
    deleteSocketInfo(&(m->mic_rcv));
    deleteSocketInfo(&(m->mic_send));
    deleteSocketInfo(&(m->send));

    // close semaphores
    CloseHandle(m->sendSem);
    CloseHandle(m->sendCompleteSem);

    // close threads
    TerminateThread(m->mic_rcv_thr, 0);
    TerminateThread(m->mic_send_thr, 0);
    TerminateThread(m->send_thr, 0);

    CloseHandle(m->mic_rcv_thr);
    CloseHandle(m->mic_send_thr);
    CloseHandle(m->send_thr);

    //delete session from map
    WaitForSingleObject(sessionsSem, INFINITE);
    SESSIONS.erase(c);
    ReleaseSemaphore(sessionsSem, 1, NULL);

    //close this thread
    ExitThread(0);
}

DWORD WINAPI micSendThread(LPVOID lpParameter)
{
    return TRUE;
}
DWORD WINAPI micRcvThread(LPVOID lpParameter)
{
    return TRUE;
}
