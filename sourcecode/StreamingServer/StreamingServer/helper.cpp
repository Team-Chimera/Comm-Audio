#include "stdafx.h"
#include "helper.h"

LPCWSTR SONG_DIR = L"..\\songs\\*.*";
std::string song_dir = "..\\songs\\";

using namespace std;
/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: loadFile
--
-- DATE: March 23, 2015
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Jeff Bayntun
--
-- PROGRAMMER: Jeff Bayntun
--
-- INTERFACE: long loadFile(const char* file_name, char** file_out)
        file_name: pointer to null terminated char* with file path and name
        file_out: will contain double pointer to contents of the file
--
-- RETURNS: -1 on failure, else the size of *fileout
--
-- NOTES:
----------------------------------------------------------------------------------------------------------------------*/
long loadFile(const char* file_name, char** file_out)
{
    long size;
    ifstream input;

    string to_open = song_dir + file_name;

    input.open( to_open, ios::binary | ifstream::ate );
    if(!input.is_open())
    {
        printf("Error opening in put file %s, aborting send", file_name);
        return -1;
    }

    size = input.tellg();
    input.seekg(ios_base::beg);

    *file_out = new char[size];
    input.read(*file_out, size);
    return size;
}

using namespace std;
/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: createWorkerThread
--
-- DATE: January 18, 2015
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Jeff Bayntun
--
-- PROGRAMMER: Jeff Bayntun
--
-- INTERFACE: bool createWorkerThread(LPTHREAD_START_ROUTINE routine, HANDLE* hThread, LPVOID param, DWORD flags)
--  routine: function this thread will start in
    hTHread: out param, handle value for this thread
    param: void* to pass to thread
    flats: any flags for thread creation
--
-- RETURNS: false on failure to create thread, else true
--
-- NOTES:
----------------------------------------------------------------------------------------------------------------------*/

bool createWorkerThread(LPTHREAD_START_ROUTINE routine, HANDLE* hThread, LPVOID param, DWORD flags)
{
    if ((*hThread = CreateThread(NULL, 0, routine, param, flags, 0)) == NULL)
    {
        printf("CreateThread failed with error %d\n", GetLastError());
        return false;
    }
    return true;
}




/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: Split
--
-- DATE: January 18, 2015
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Jeff Bayntun
--
-- PROGRAMMER: Jeff Bayntun
--
-- INTERFACE: void Split( std::string list, std::vector<std::string>& v )
-- list: delimiter seperated string
-- &v reference to a vector where parsed tokens will be stored
--
-- RETURNS: false if there are non-digit values in the string, else true
--
-- NOTES:
----------------------------------------------------------------------------------------------------------------------*/
void Split( string list,vector<string>& v )
{
    size_t pos = 0;
    string delimiter(1, DELIM);
    string token;
    while ( (pos = list.find( delimiter ) ) != string::npos )
    {
        token = list.substr( 0, pos );
        v.emplace_back(token);
        list.erase( 0, pos + delimiter.length() );
        token.clear();
    }

    return;
}

/*******************************************************************
** Function: createControlString
**
** Date: March 14th, 2015
**
** Revisions:
**
**
** Designer: Rhea Lauzon
**
** Programmer: Rhea Lauzon
**
** Interface:
**			void createControlString(ctrlMessage &msg, string &str)
**              ctrlMessage &msg -- Message to create from
**              string &str -- Reference of string to place data in
**
**
** Returns:
**			void
**
** Notes:
** Converts a control string structure to a string to send
*******************************************************************/
void createControlString(ctrlMessage &msg, string &str)
{
    //add the type to the string
    stringstream ss;
    ss << msg.type << "~";

    //loop through all the data in the array
    for (int i = 0; i < (int) msg.msgData.size(); i++)
    {
        ss << msg.msgData[i] << "|";
    }

    str = ss.str();

}

/*******************************************************************
** Function: parseControlString
**
** Date: March 14th, 2015
**
** Revisions:
**
**
** Designer: Rhea Lauzon
**
** Programmer: Rhea Lauzon
**
** Interface:
**			void parseControlString(string str, ctrlMessage *msg)
**              string str -- string received
**              ctrlMessage *msg - The control message received
**
** Returns:
**			int
**
** Notes:
** Handles all control messages received by the control channel
*******************************************************************/
void parseControlString(string str, ctrlMessage *msg)
{
    //get the type of the message
    int type = str.find('~');
    msg->type = static_cast<messageType> (atoi(str.substr(0, type).c_str()));

    //chunk off the message's type
    str = str.substr(type +1, str.length());

    //parse the message's data out of the remaining string
    while(str.length() > 0)
    {
        int endData = str.find('|');
        msg->msgData.emplace_back(str.substr(0, endData));

        str = str.substr(endData + 1, str.length());
    }

}

/*******************************************************************
** Function: displayError
**
** Date: March 12th, 2015
**
** Revisions:
**
**
** Designer: Julian Brandrick
**
** Programmer: Julian Brandrick
**
** Interface:
**			void displayError(char *errStr, int errCode)
** 
** Parameters:
**          errStr - Error string to be displayed
**          errCode - Error code that was thrown
**
** Returns:
**			void
**
** Notes:
**  This function prints a formatted error string and code to 
**  stderr.
*******************************************************************/
void displayError(char *errStr, int errCode)
{
    fprintf(stderr, "Error>%s: %d\n", errStr, errCode);
}

/*******************************************************************
** Function: displayOutput
**
** Date: March 12th, 2015
**
** Revisions:
**
**
** Designer: Julian Brandrick
**
** Programmer: Julian Brandrick
**
** Interface:
**			void displayOutput(char *outStr)
** 
** Parameters:
**          outStr - Output string to be displayed
**
** Returns:
**			void
**
** Notes:
**  This function prints a formatted output string to stdout.
*******************************************************************/
void displayOutput(char *outStr)
{
    printf("Output>%s\n", outStr);
}



