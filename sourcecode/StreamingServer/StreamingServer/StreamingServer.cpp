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
	
	if(!loadSongList())
	{
		cout << "error loading songs, aborting" << endl;
		return 55;
	}
	
    cout << "Id of Main thread " << GetCurrentThreadId() << endl;
	 initWSA(&wsadata);
    // Julian's multicast...
    createWorkerThread(startMulticastThread, &multicastThread, (LPVOID *) &songList, 0);
    AcceptThread();

    return 0;

}


bool loadSongList()
{
    cout << endl << "Loading Songs" << endl;
    HANDLE hFind;
    WIN32_FIND_DATA data;
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
        string temp = CW2A(data.cFileName);
        songList.push_back(temp);
      }
  } while (FindNextFile(hFind, &data));

  FindClose(hFind);
  if(songList.empty())
  {
      cout << "no songs in song directory" << endl;
      return false;
  }

  for(string s : songList)
  {
      cout << s << endl;
  }

  return true;
}