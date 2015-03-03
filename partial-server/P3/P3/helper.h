#ifndef HELPER_H
#define HELPER_H

#include <Windows.h>
#include <stdio.h>

#include <vector>

const char DELIM = ':';

void Split( std::string list, std::vector<std::string>& v );
bool createWorkerThread(LPTHREAD_START_ROUTINE routine, HANDLE* hThread, LPVOID param, DWORD flags = 0);


#endif