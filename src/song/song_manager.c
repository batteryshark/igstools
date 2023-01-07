#include <stdio.h>
#include <string.h>


#include "song_defs.h"

#include "song_recfile.h"
#include "song_event.h"
#include "song_settings.h"


#include "../pm_funcs.h"

#include "song_manager.h"

SongSettings settings;



void LoadSongEvent(PRecFile rec_file, PSongEvent song_event, PSongSettings settings){
    ParseRecHeader(rec_file,song_event);
    song_event->p1_num_cursor_events = ParseCursorEvents(rec_file->p1_events,song_event->p1_event,song_event->tempo,settings->p1_speed);
    song_event->p2_num_cursor_events = ParseCursorEvents(rec_file->p2_events,song_event->p2_event,song_event->tempo,settings->p2_speed);
    song_event->num_sound_events = ParseSoundEvents(rec_file->sound_events,song_event->sound_event,song_event->tempo);
    
}

void SongManager_Init(void* setting_data){
    printf("SongManager_Init\n");
    // Copy our Song Settings Data
    memcpy(&settings,setting_data,sizeof(SongSettings));
    
    // Initialize Our Song State Data
    memset(&song_state,0,sizeof(SongState));    
    

    

    memset(&p1_judge,0,sizeof(PlayerJudge));
    memset(&p2_judge,0,sizeof(PlayerJudge));    
    song_state.cmd = A27_SONGMODE_MAINGAME_SETTING;
}
