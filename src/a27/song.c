#include <stdio.h>
#include "../song/song_manager.h"
#include "song.h"

#define STATE_BUFFER_SIZE 0xA00
#define RESULT_BUFFER_SIZE 0x50

// TODO: Load Real Song On Machine and Check
// -- Unimplemented Function 0 --
void Song_UploadPlaybackHeader(const unsigned char* in_data, struct A27_Read_Message* msg){
    // TODO: Upload Logic
    // This Effectively means OK?
    printf("Upload Playback Header:\n");
    for(int i=0;i<148;i++){
        printf("%02x",in_data[i]);
    }
    printf("\n");
    msg->dwBufferSize = 4;
    msg->data[0] = 2;
}

// -- Unimplemented Function 1 --
void Song_UploadPlaybackBody(const unsigned char* in_data, struct A27_Read_Message* msg){    
    // Do Nothing for Now
    // TODO: Upload Logic
    // This Effectively means OK?
        printf("Upload Playback Data:\n");
    for(int i=0;i<4004;i++){
        printf("%02x",in_data[i]);
    }
    printf("\n");
    msg->dwBufferSize = 4;
    msg->data[0] = 2;
}

// -- Function 3: This tells the card what song we're playing and sets our session options.
// -- This is called twice, so don't do any one-shot stuff in here.
void Song_MainGameSetting(const unsigned char* in_data, struct A27_Read_Message* msg){
    msg->dwBufferSize = SongManager_Init((void*)in_data,(void*)msg->data);
}

// -- Function 4: This is when we start initializing the song state stuff like setting note counters, etc.
void Song_MainGameWaitStart(const unsigned char* in_data, struct A27_Read_Message* msg){
    msg->dwBufferSize = SongManager_Reset((void*)msg->data);
}

// -- Function 5: This is a one-shot function that starts the song timer and is likely the official start of the song.
void Song_MainGameStart(const unsigned char* in_data, struct A27_Read_Message* msg){
    msg->dwBufferSize = SongManager_Start((void*)msg->data);
}
// -- Function 6: This is called every frame to update the song state
void Song_MainGameProcess(const unsigned char* in_data, struct A27_Read_Message* msg){    
    msg->dwBufferSize = SongManager_Update((void*)msg->data);  
}

// -- Function 9: This stops the song and goes to results. I think Function 7 also does that but we'll deal later.
void Song_ResultProcess(const unsigned char* in_data, struct A27_Read_Message* msg){
    SongManager_Stop();    
    msg->dwBufferSize = SongManager_GetResult((void*)msg->data);
}



 
