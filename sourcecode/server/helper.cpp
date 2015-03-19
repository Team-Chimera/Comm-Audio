#include "helper.h"

using namespace std;

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
