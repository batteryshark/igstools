// Functions Specific to PercussionMaster ELF and May Need Modification if Another ELF is Used.
#include <string.h>

#include "pm_funcs.h"
#include "song/song_defs.h"

unsigned short (*song_wSongNameGet)(unsigned short chart_id) = (void*)0x08077188;
unsigned char (*song_bSongRankGet)(unsigned short chart_id) = (void*)0x080771A0;
const char** g_pchaSongName = (void*)0x08171CE0;
const char** g_pchaSongRank = (void*)0x08171D94;

int GenerateEventFilename(unsigned int song_mode, unsigned int chart_id, char* event_file_path){
    switch(song_mode){
     case SONG_MODE_OPENING:
         strcat(event_file_path,"Opening.rec");
         return 1;
     case SONG_MODE_HOWTOPLAY:
         strcat(event_file_path,"HowToPlay.rec");
         return 1;
     case SONG_MODE_STAFF:
         strcat(event_file_path,"Staff.rec");
         return 1;
     default:
         break;     
    }
    strcat(event_file_path,g_pchaSongName[song_wSongNameGet(chart_id)]);
    strcat(event_file_path,"_");
    strcat(event_file_path,g_pchaSongRank[song_bSongRankGet(chart_id)]);
    strcat(event_file_path,".rec"); 
    return 1;
}
