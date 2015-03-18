#ifndef CONTROLMESSAGE
#define CONTROLMESSAGE

#include <iostream>
#include <vector>
#include <string>

enum messageType
{
    END_CONNECTION,
    MIC_CONNECTION,
    SONG_REQUEST,
    PLAY_SONG,
    SAVE_SONG,
    LIBRARY_INFO,
    NOW_PLAYING,
    END_SONG,
    CURRENT_LISTENERS

};
typedef enum messageType messageType;

struct ctrlMessage
{
    messageType type;
    std::vector<std::string> msgData;
};
typedef struct ctrlMessage ctrlMessage;


#endif // CONTROLMESSAGE
