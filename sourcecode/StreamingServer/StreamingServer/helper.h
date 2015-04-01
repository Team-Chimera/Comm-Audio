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

const LPCWSTR SONG_DIR = L"..\\songs\\*.*";
const std::string song_dir = "..\\songs\\";


void parseControlString(std::string str, ctrlMessage *msg);
void createControlString(ctrlMessage &msg, std::string &str);
void Split( std::string list, std::vector<std::string>& v );
bool createWorkerThread(LPTHREAD_START_ROUTINE routine, HANDLE* hThread, LPVOID param, DWORD flags = 0);
long loadFile(const char* file_name, char** file_out);

const static char DELIM = ':';

#endif
