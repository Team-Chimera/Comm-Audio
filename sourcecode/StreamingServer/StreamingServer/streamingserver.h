#ifndef SERV_STREAM_H
#define SERV_STREAM_H


#include <stdio.h>
#include <iostream>
#include <map>
#include <string>
#include "newsession.h"
#include "network.h"
#include "atlstr.h"

#pragma comment(lib, "ws2_32.lib")

bool makeSharedSems();
bool loadSongList();

#endif
