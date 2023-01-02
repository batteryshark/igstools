#include <stdio.h>
#include "song_manager.h"
#include "song.h"

#define STATE_BUFFER_SIZE 0xA00
#define RESULT_BUFFER_SIZE 0x50

// -- Unimplemented Function 0 --
void Song_UploadPlaybackHeader(const unsigned char* in_data, struct A27_Read_Message* msg){
    // TODO: Upload Logic
    // This Effectively means OK?
    msg->dwBufferSize = 4;
    msg->data[0] = 2;
}

// -- Unimplemented Function 1 --
void Song_UploadPlaybackBody(const unsigned char* in_data, struct A27_Read_Message* msg){    
    // Do Nothing for Now
    // TODO: Upload Logic
    // This Effectively means OK?
    msg->dwBufferSize = 4;
    msg->data[0] = 2;
}

// -- Function 3: This tells the card what song we're playing and sets our session options.
// -- This is called twice, so don't do any one-shot stuff in here.
void Song_MainGameSetting(const unsigned char* in_data, struct A27_Read_Message* msg){
    SongManager_Init((void*)in_data);
    GetSongState((void*)msg->data);
    msg->dwBufferSize = STATE_BUFFER_SIZE;
}

// -- Function 4: This is when we start initializing the song state stuff like setting note counters, etc.
void Song_MainGameWaitStart(const unsigned char* in_data, struct A27_Read_Message* msg){
    SongManager_Reset();
    GetSongState((void*)msg->data);
    msg->dwBufferSize = STATE_BUFFER_SIZE;
}

// -- Function 5: This is a one-shot function that starts the song timer and is likely the official start of the song.
void Song_MainGameStart(const unsigned char* in_data, struct A27_Read_Message* msg){
    SongManager_Start();
    GetSongState((void*)msg->data);
    msg->dwBufferSize = STATE_BUFFER_SIZE;
}
// -- Function 6: This is called every frame to update the song state
void Song_MainGameProcess(const unsigned char* in_data, struct A27_Read_Message* msg){
    SongManager_Update((void*)msg);
    msg->dwBufferSize = STATE_BUFFER_SIZE;    
}

// -- Function 9: This stops the song and goes to results. I think Function 7 also does that but we'll deal later.
void Song_ResultProcess(const unsigned char* in_data, struct A27_Read_Message* msg){
    SongManager_Stop();
    GetSongResult((void*)in_data, (void*)msg->data);    
    msg->dwBufferSize = RESULT_BUFFER_SIZE;
}



 
