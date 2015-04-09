/*---------------------------------------------------------------------------------------------
-- SOURCE FILE: newsession.cpp -
--
-- PROGRAM: StreamingServer.exe
--
-- DATE: March 22, 2015
--
-- Interface:
--   void AcceptThread();
--    void sendSongList(SOCKET c);
--    bool createSession(new_session* ns);
--    void sessionCleanUp(SOCKET s);
--    DWORD WINAPI controlThread(LPVOID lpParameter);
--    void CALLBACK controlRoutine(DWORD Error, DWORD BytesTransferred, LPWSAOVERLAPPED Overlapped, DWORD InFlags);
--    void sendNowPlaying(MetaData *);
--    void updateNewUser(SOCKET c);
--    void transmitSong(SOCKET s, std::string song);
--    DWORD WINAPI sendTCPSong(LPVOID lpParameter);
--    void sendToAll(std::string);
--    void sendUserList(SOCKET c);
--    void sendSongDone(SOCKET s, int);
--    void signalUserChanged();
--	void swapSong();

-- REVISIONS: (Date and Description)
--
-- DESIGNER: Jeff Bayntun
--
-- PROGRAMMER: Jeff Bayntun
--------------------------------------------------------------------------------------------*/

#include "stdafx.h"
#include "newsession.h"
#include <iostream>
#include "helper.h"
#include <mmsystem.h>
#include <vlc/vlc.h>
#include <vlc/libvlc.h>
#include "music.h"
#include "unicast.h"

using namespace std;

int userSemValue;
vector<string> songList;
HANDLE sessionsSem;

std::map<SOCKET, new_session*> SESSIONS;

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
**			void AcceptThread()
**
**
** Returns:
**
** Notes: listens and waits for incoming connections indefinetely.
            when a client connects, creates a session and spawns
            a thread to handle them.
*******************************************************************/
void AcceptThread()
{
    SOCKET Accept, temp;
    sockaddr peer;
    int peer_len;
    new_session* ns;

    if( (sessionsSem = CreateSemaphore(NULL, 1, 1, NULL)) == NULL)
    {
        printf("error creating sessionSem\n");
        return;
    }

    if(!openListenSocket(&Accept, SERVER_TCP_LISTEN_PORT) )
        return;


        while(TRUE)
       {
           peer_len = sizeof(peer);
           ZeroMemory(&peer, peer_len);

           temp = accept(Accept, &peer, &peer_len);
           ns = new new_session;
           ns->s = temp;

            getpeername(temp, &peer, &peer_len);
            sockaddr_in* s_in = (struct sockaddr_in*)&peer;
            char* temp_addr = inet_ntoa( s_in->sin_addr );

            ns->ip.assign(temp_addr);
            createSession(ns);
       }

    return;
}

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
** Notes: Creates the a worker thread for a new session
*******************************************************************/
bool createSession(new_session* ns)
{
    HANDLE h;
    cout << endl << "A New Client Has Joined with IP: " << ns->ip << endl;

    WaitForSingleObject(sessionsSem, INFINITE);
    SESSIONS.insert(make_pair(ns->s, ns));
    ReleaseSemaphore(sessionsSem, 1, 0);

    createWorkerThread(controlThread, &h, ns, 0);

    return true;
}


/*******************************************************************
** Function:  controlThread()
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
**			DWORD WINAPI controlThread(LPVOID lpParameter)
            lpParameter is new_session* for this session
**
**
** Returns:
**			FALSE on failure
**
** Notes: each session has its own control thread.  This is used
**  to coordinate other threads and activities.  waits for tcp action
    on the control socket, or semaphore notifications for new 
    users or completed song transfers
*******************************************************************/
DWORD WINAPI controlThread(LPVOID lpParameter)
{
    new_session* ns = (new_session*) lpParameter;
    SOCKET s = ns->s;

    LPSOCKET_INFORMATION SI;
    SI = createSocketInfo(s);


    LPSOCKET_INFORMATION si = SI;

    if( (ns->transferCompleteSem = CreateSemaphore(NULL, 0, 1, NULL)) == NULL)
    {
        printf("error creating transferComplete\n");
        return FALSE;
    }

    if( (ns->userChangeSem = CreateSemaphore(NULL, 0, 1, NULL)) == 0)
    {
        printf("error creating userChangeSem\n");
        return FALSE;
    }

    DWORD RecvBytes, result, flags;
    int handles = 2;
    HANDLE* waitHandles = new HANDLE[handles];
    waitHandles[0] = ns->userChangeSem;
    waitHandles[1] = ns->transferCompleteSem;

    // send song list and currently playing, then update all users about the new user
    sendSongList(s);
    updateNewUser(s);
    signalUserChanged();
    
    //post rcv call waiting for data
    if ( WSARecv( s, &(si->DataBuf), 1, &RecvBytes, &flags, &(si->Overlapped), controlRoutine ) == SOCKET_ERROR)
      {
         if (WSAGetLastError() != WSA_IO_PENDING && si)
         {
            printf("WSARecv() failed with error %d, control thread\n", WSAGetLastError());
            SOCKET aaa = si->Socket;
            deleteSocketInfo(si);
            sessionCleanUp(aaa);
         }
      } 

    // get into alertable state
    while(1)
    {
        result = WaitForMultipleObjectsEx(handles, waitHandles, FALSE, INFINITE, TRUE);
        if(result == WAIT_FAILED)
        {
            perror("error waiting for multiple objects");
            sessionCleanUp(si->Socket);
        }

        if (result == WAIT_IO_COMPLETION)
         {
            continue;
         }

         switch(result -  WAIT_OBJECT_0)
         {
            case 0:
                 // change in user list, access the list and send it
                sendUserList(si->Socket);

                userSemValue--;
                 break;
            case 1:
                //transfer of a song is completed, send message for that
                sendSongDone(si->Socket, ns->type);
                break;
         default:
                //error of somekind, clean up sessions and exit
                sessionCleanUp(si->Socket);
         }
    }

    return FALSE;
}
/*******************************************************************
** Function:  sendSongList()
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
**			void sendSongList(SOCKET c)
**          c: socket to send on
**
** Returns:
**
** Notes: sends the list of songs in the library using the 
            given socket.
*******************************************************************/
void sendSongList(SOCKET c)
{
    if(songList.empty())
        return;

    string temp;
    ctrlMessage message;

    message.msgData = songList;
    message.type = LIBRARY_INFO;
    createControlString(message, temp);
    string to_send = "********************************************" + temp;

    sendTCPMessage(&c, to_send, DATA_BUFSIZE);

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
** Notes: closes semaphores and ends this sessions control thread
           updates other users with new client list
*******************************************************************/
void sessionCleanUp(SOCKET s)
{
    //delete session from map
    WaitForSingleObject(sessionsSem, INFINITE);
    new_session* ns = SESSIONS.at(s);

    if(ns)
    {
        CloseHandle(ns->transferCompleteSem);
        CloseHandle(ns->userChangeSem);
        int inny = SESSIONS.erase(s);
    }
    ReleaseSemaphore(sessionsSem, 1, NULL);

    //signal other sessions to send the updated userlist
    signalUserChanged();

    //close this thread
    ExitThread(0);
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
** Programmer: Jeff Bayntun, Rhea Lauzon
**
** Interface:
**			void CALLBACK controlRoutine(DWORD Error, DWORD BytesTransferred, LPWSAOVERLAPPED Overlapped, DWORD InFlags)
            Error: error value
            BytesTransferred: number of bytes transferred
            Overlapped: Overlapped structure for transfer
            InFlags: flags set
** Returns:
**		 void
**
** Notes: control thread for a session in response to WSA socket callbacks.
            takes action based on control message received
*******************************************************************/
void CALLBACK controlRoutine(DWORD Error, DWORD BytesTransferred, LPWSAOVERLAPPED Overlapped, DWORD InFlags)
{

    DWORD SendBytes, RecvBytes;
   DWORD Flags;
   string message_rcv;
   ctrlMessage ctrl;


   // Reference the WSAOVERLAPPED structure as a SOCKET_INFORMATION structure
   LPSOCKET_INFORMATION SI = (LPSOCKET_INFORMATION) Overlapped;

   if (Error != 0)
   {
   }

   if (BytesTransferred == 0) // nothing sent or rcvd...
   {
   }

   if (Error != 0 || BytesTransferred == 0)
   {
       SOCKET sss = SI->Socket;
       deleteSocketInfo(SI);
       sessionCleanUp(sss);
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
			  cout << "==========================================" << endl;
              cout << "mic connection from socket " << SI->Socket << endl;
			  cout << "==========================================" << endl << endl << endl;
              break;
          }
          //song has been requested for unicast send to this client
          case SONG_REQUEST:
          {
			cout << "==========================================" << endl;
			cout << "song request from socket " << SI->Socket << endl;
			cout << "==========================================" << endl;
			// get ns
			new_session* ns;
			WaitForSingleObject(sessionsSem, INFINITE);
			ns = SESSIONS.at(SI->Socket);
			ReleaseSemaphore(sessionsSem, 1, 0);

			ClientData *cd = new ClientData;
			cd->session = ns;
			cd->ip = ns->ip;
			cd->song = ctrl.msgData[0];

			HANDLE thread;
			createWorkerThread(startUnicast, &thread, (LPVOID) cd, 0);
            break;
          }

          //song has been requested for tcp send to this client
          case SAVE_SONG:
          {
			cout << "==========================================" << endl;
            cout << "download request from socket " << SI->Socket << endl;
			cout << "==========================================" << endl;
            transmitSong(SI->Socket, ctrl.msgData[0]);
              break;
          }
          case END_CONNECTION:
          default:
              {
                   SOCKET aaa = SI->Socket;
                   deleteSocketInfo(SI);
                   sessionCleanUp(aaa);
              }
      }

       SI->bytesToSend = 0; // send back same size, that is what they're expecting
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
            printf("WSARecv() failed with error %d, ctrl routine\n", WSAGetLastError());
            return;
         }
      }
   }
}

/*******************************************************************
** Function:  updateNewUser()
**
** Date: March 22th, 2015
**
** Revisions:
**
**
** Designer: Rhea Lauzon
**
** Programmer: Rhea Lauzon
**
** Interface:
**			void updateNewUser(SOCKET c)
            c: socket to send on
** Returns:
**		 void
**
** Notes: sends currently playing song info over given socket
*******************************************************************/
void updateNewUser(SOCKET c)
{

	ctrlMessage message;
	string temp;

	
	MetaData *data = new MetaData;
	fetchMetaData(data);
	//create the now playing control message
	stringstream ss;

	if (data->title == NULL)
	{
		ss << "Unknown Song^";
	}
	else
	{
		ss << data->title << "^";
	}

	//add the artist
	if (data->artist == NULL)
	{
		ss << "Unknown Artist^";
	}
	else
	{
		ss << data->artist << "^";
	}

	//add the album
	if (data->album == NULL)
	{
		ss << "Unknown Album^";
	}
	else
	{
		ss << data->album << "^";
	}

	message.msgData.push_back(ss.str());
    message.type = NOW_PLAYING;

    createControlString(message, temp);
    string to_send = "********************************************" + temp;

	//send the message to the client
	sendTCPMessage(&c, to_send, DATA_BUFSIZE);
	
	delete data;
	
}

/*******************************************************************
** Function:  sendUserList()
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
**			void sendUserList(SOCKET c)
            c: socket to send on
** Returns:
**		 void
**
** Notes: sends user List to current socket
*******************************************************************/
void sendUserList(SOCKET c)
{
    vector<string> users;
    string temp;
    ctrlMessage message;
    new_session* ns;

    WaitForSingleObject(sessionsSem, INFINITE);
    map<const SOCKET, new_session*>::iterator &it = SESSIONS.begin();
    while(it != SESSIONS.end())
    {
        ns = it->second;
        if(it->first == c)
        {
            users.push_back("Me: " + ns->ip);
        }
        else
        {
            users.push_back(ns->ip);
        }
        it++;
    }
    ReleaseSemaphore(sessionsSem, 1, 0);

    if(users.empty())
        return;

    
    message.msgData = users;
    message.type = CURRENT_LISTENERS;
    createControlString(message, temp);

    string to_send = "********************************************" + temp;

    //call send function
    sendTCPMessage(&c, to_send, DATA_BUFSIZE);
}


/*******************************************************************
** Function:  sendNowPlaying()
**
** Date: March 22th, 2015
**
** Revisions:
**
**
** Designer: Rhea Lauzon
**
** Programmer: Rhea Lauzon
**
** Interface:
**			void sendNowPlaying(MetaData *data)
            metadata for song
** Returns:
**		 void
**
** Notes: creates now playing list to send to multicast clients
*******************************************************************/
void sendNowPlaying(MetaData *data)
{
	string temp;
    ctrlMessage message;

	//create the control message
	stringstream ss;

	//add the song name
	if (data->title == NULL)
	{
		ss << "Unknown Song^";
	}
	else
	{
		ss << data->title << "^";
	}

	//add the artist
	if (data->artist == NULL)
	{
		ss << "Unknown Artist^";
	}
	else
	{
		ss << data->artist << "^";
	}

	//add the album
	if (data->album == NULL)
	{
		ss << "Unknown Album^";
	}
	else
	{
		ss << data->album << "^";
	}
	
	message.msgData.push_back(ss.str());
    message.type = NOW_PLAYING;

    createControlString(message, temp);

    string to_send = "********************************************" + temp;

    //call send function
	sendToAll(to_send);
}

/*******************************************************************
** Function:  sendToAll()
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
**			void sendToAll(string message)
            message to send
** Returns:
**		 void
**
** Notes: sends a string to all clients.
        todo:  make new function that is thread safe
*******************************************************************/
void sendToAll(string message)
{
	std::map<SOCKET, new_session *>::iterator it;

	for( it = SESSIONS.begin(); it != SESSIONS.end(); it++)
	{
		//send it all clients
		sendTCPMessage((SOCKET *)(&(it->first)), message, message.size());
	}
}

/*******************************************************************
** Function:  transmitSong()
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
**			void transmitSong(SOCKET s, string song)
            s: socket to send on
            song: title of song
** Returns:
**		 void
**
** Notes: creates a thread to send a song via TCP to a song
*******************************************************************/
void transmitSong(SOCKET s, string song)
{
    // get ns
    new_session* ns;
    HANDLE thread;

    WaitForSingleObject(sessionsSem, INFINITE);
    ns = SESSIONS.at(s);
    ReleaseSemaphore(sessionsSem, 1, 0);

    // put song in ns
    ns->song.assign(song);

    // start thread with ns as param to do the transfer.
    createWorkerThread(sendTCPSong, &thread, ns, 0);
}

/*******************************************************************
** Function:  sendTCPSong()
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
**			DWORD WINAPI sendTCPSong(LPVOID lpParameter)
            param is new_session* for this session
** Returns:
**		 void
**
** Notes: thread function that handles the sending of a song
            to a client via TCP
*******************************************************************/
DWORD WINAPI sendTCPSong(LPVOID lpParameter)
{
    new_session* ns = (new_session*) lpParameter;
    char* temp;
    string to_send;
    long file_size;

    //load file for song
    if((file_size = loadFile(ns->song.c_str(), &temp)) == -1)
    {   cout << "Error loading file " << ns->song << " for tcp send" << endl;
        return FALSE;
    }
    
    SOCKET socket;
    //Sleep(2000);
    if(!openTCPSend(&socket, CLIENT_TCP_PORT, ns->ip) )
    {
        cout << "couldn't connect to client rcv" << endl;
        return FALSE;
    }

    //send song
    //Sleep(500);
    sendTCPMessage(&socket, temp, file_size, DATA_BUFSIZE);

   //signal control thread that song is completed
	ns->type = 0;
    ReleaseSemaphore(ns->transferCompleteSem, 1, 0);

    //exit thread
    closesocket(socket);
    return TRUE;
}

/*******************************************************************
** Function:  sendSongDone()
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
**			void sendSongDone(SOCKET s, int mode)
            socket to send on
            mode, tcp or udp song
** Returns:
**		 void
**
** Notes: send message to user that file transfer is complete
*******************************************************************/
void sendSongDone(SOCKET s, int mode)
{
    string temp;
    ctrlMessage message;

	stringstream ss;
	ss << mode;

    message.msgData.push_back(ss.str());
    message.type = END_SONG;
    createControlString(message, temp);
    string to_send = "********************************************" + temp;

    sendTCPMessage(&s, to_send, DATA_BUFSIZE);
}


/*******************************************************************
** Function:  swapSong
**
** Date: April 8th, 2015
**
** Revisions:
**
**
** Designer: Rhea Lauzon
**
** Programmer: Rhea Lauzon
**
** Interface:
**			void swapSong()
** Returns:
**		 void
**
** Notes: sends a "Swap song" message to all clients
** in order to prevent the clients from chasing the buffer.
*******************************************************************/
void swapSong()
{
	//sends an end message to all the clients
	string temp;
    ctrlMessage message;

	
	message.msgData.push_back("2");
    message.type = END_SONG;

    createControlString(message, temp);

    string to_send = "********************************************" + temp;

	sendToAll(to_send);

}

/*******************************************************************
** Function:  signalUserChanged()
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
**			void signalUserChanged()
** Returns:
**		 void
**
** Notes: signals all current sessions that the userList has
            changed so they can update their clients
*******************************************************************/
void signalUserChanged()
{
    new_session* ns;

    WaitForSingleObject(sessionsSem, INFINITE);

    auto it = SESSIONS.begin();
    while(it != SESSIONS.end())
    {
        ns = it->second;
        ReleaseSemaphore(ns->userChangeSem, 1, 0);
        it++;
    }

    ReleaseSemaphore(sessionsSem, 1, 0);
}