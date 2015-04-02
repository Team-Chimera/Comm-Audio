#ifndef HELPER_H
#define HELPER_H

#define NOMINMAXs

#include <Windows.h>
#include <stdio.h>
#include <string>
#include <sstream>
#include <fstream>
#include "../../client/controlMessage.h"

#include <vector>

extern LPCWSTR SONG_DIR;
extern std::string song_dir;


void parseControlString(std::string str, ctrlMessage *msg);
void createControlString(ctrlMessage &msg, std::string &str);
void Split( std::string list, std::vector<std::string>& v );
bool createWorkerThread(LPTHREAD_START_ROUTINE routine, HANDLE* hThread, LPVOID param, DWORD flags = 0);
long loadFile(const char* file_name, char** file_out);

void displayError(char *errStr, int errCode);
void displayOutput(char *outStr);

const static char DELIM = ':';

#endif
