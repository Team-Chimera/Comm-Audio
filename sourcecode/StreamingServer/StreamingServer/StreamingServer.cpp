// StreamingServer.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "multicast.h"
#include "streamingserver.h"


using namespace std;

WSAData wsadata;
HANDLE acceptThread;
HANDLE multicastThread;

/*******************************************************************
** Function: main()
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
**			argc: the number of command line arguments
**          argv: array with values for each command line argument
**
**
** Returns:
**			0 on success
**
** Notes:
*******************************************************************/
int main(int argc, char *argv[])
{
	/*
	if(!makeSharedSems() || !loadSongList())
	{
		cout << "error making semaphores, aborting" << endl;
		return 55;
	}
	*/


	 initWSA(&wsadata);
    // Julian's multicast...
    startMulticastThread(&multicastThread);
    
    AcceptThread();

    return 0;

}

/*******************************************************************
** Function: makeSharedSems()
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
**			bool makeSharedSems()
**
**
** Returns:
**			true on success
**
** Notes:
** create semaphores that will be used by all session threads, including
 * for changes in the list of users or the currenly multicasted song
*******************************************************************/
bool makeSharedSems()
{
    if( (userAccessSem = CreateSemaphore(NULL, 1, 1, NULL)) == NULL)
    {
        printf("error creating sessionSem\n");
        return false;
    }

    if( (songAccessSem = CreateSemaphore(NULL, 1, 1, NULL)) == NULL)
    {
        printf("error creating sessionSem\n");
        return false;
    }

    if( (newSongSem = CreateSemaphore(NULL, 0, MAX_SESSIONS, NULL)) == NULL)
    {
        printf("error creating sessionSem\n");
        return false;
    }

    if( (userChangeSem = CreateSemaphore(NULL, 0, MAX_SESSIONS, NULL)) == NULL)
    {
        printf("error creating sessionSem\n");
        return false;
    }

    return true;
}

bool loadSongList()
{
    HANDLE hFind;
    WIN32_FIND_DATA data;
    stringstream ss;
    int count = 0;

    hFind = FindFirstFile(SONG_DIR, &data);
    if (hFind == INVALID_HANDLE_VALUE)
    {
        cout << "invalid song directory" << endl;
        return false;
    }


  do 
  {
      if(count++ > 1)
      {
        ss << data.cFileName;
        songList.push_back(ss.str());
      }
  } while (FindNextFile(hFind, &data));

  FindClose(hFind);
  if(songList.empty())
  {
      cout << "no songs in song directory" << endl;
      return false;
  }

  return true;
}