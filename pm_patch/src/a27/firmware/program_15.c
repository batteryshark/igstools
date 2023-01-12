#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "../song/song_manager.h"
#include "../emulator.h"

// Program 15: Song State Processing
void A27_Program_15(PA27WriteMessage req, PA27ReadMessage res){
    res->header.system_mode = A27_MODE_SONG;
    unsigned short song_subcmd = *(unsigned short*)req->data;
    switch(song_subcmd){
        // Upload a recorded header for playback. [Dev Only]
        case A27_SONGMODE_PLAYBACK_HEADER:
            res->header.data_size = SongManager_Record_Header((void*)req->data,(void*)res->data); 
            break;
        // Upload recorded events for playback. [Dev Only]
        case A27_SONGMODE_PLAYBACK_BODY:
            res->header.data_size = SongManager_Record_Body((void*)req->data,(void*)res->data);
            break;
        case A27_SONGMODE_2:
            res->header.data_size = SongManager_SongMode_2((void*)req->data,(void*)res->data);
            break;
        // -- Function 3: This tells the card what song we're playing and sets our session options.
        // -- This is called twice, so don't do any one-shot stuff in here.
        case A27_SONGMODE_MAINGAME_SETTING:
            res->header.data_size = SongManager_Init((void*)req->data,(void*)res->data);
            break;
        // -- Function 4: This is when we start initializing the song state stuff like setting note counters, etc.
        case A27_SONGMODE_MAINGAME_WAITSTART:
            res->header.data_size = SongManager_Reset((void*)res->data);
        // -- Function 5: This is a one-shot function that starts the song timer and is likely the official start of the song.
        case A27_SONGMODE_MAINGAME_START:
            res->header.data_size = SongManager_Start((void*)res->data);
            break;     
        // -- Function 6: This is called every frame to update the song state            
        case A27_SONGMODE_MAINGAME_PROCESS:
            res->header.data_size = SongManager_Update((void*)res->data);
            break;
        // -- Function 9: This stops the song and goes to results. I think Function 7 also does that but we'll deal later.            
        case A27_SONGMODE_RESULT:
            res->header.data_size = SongManager_GetResult((void*)res->data);
            break;            
        default:
            printf("[A27Emu::SongProcess] Error: Unhandled Subcommand: %d\n",song_subcmd);
            exit(1);        
    }
}
