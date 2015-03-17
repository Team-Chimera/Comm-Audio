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
