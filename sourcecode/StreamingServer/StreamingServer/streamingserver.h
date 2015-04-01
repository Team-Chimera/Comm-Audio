#ifndef SERV_STREAM_H
#define SERV_STREAM_H


#include <stdio.h>
#include <iostream>
#include <sstream>
#include <map>
#include <string>
#include "session.h"
#include "network.h"

#pragma comment(lib, "ws2_32.lib")

bool makeSharedSems();
bool loadSongList();

#endif
