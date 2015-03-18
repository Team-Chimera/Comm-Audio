#ifndef HELPER_H
#define HELPER_H

#include <Windows.h>
#include <stdio.h>
#include <string>
#include <sstream>
#include "../client/controlMessage.h"

#include <vector>

const char DELIM = ':';

void parseControlString(std::string str, ctrlMessage *msg);
void createControlString(ctrlMessage &msg, std::string &str);
void Split( std::string list, std::vector<std::string>& v );
bool createWorkerThread(LPTHREAD_START_ROUTINE routine, HANDLE* hThread, LPVOID param, DWORD flags = 0);


#endif
