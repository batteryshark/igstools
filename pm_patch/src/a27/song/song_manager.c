#include <stdio.h>
#include <string.h>
#include "../a27.h"

#include "song_result.h"
#include "song_settings.h"
#include "song_state.h"
#include "song_event.h"
#include "song_judge.h"
#include "song_manager.h"


// Song State Management 
SongSettings settings;
SongState state;
SongJudge judge;
RecFile rec_file;
SongEvent event;
static unsigned char in_song;
unsigned char SongManager_InSong(void){return in_song;}
void SongManager_StopSong(void){in_song = 0;}
void SongManager_StartSong(void){in_song = 1;}

void ResetStateManagement(void){
    memset(&settings,0,sizeof(SongSettings));
    memset(&event,0,sizeof(SongEvent));
    memset(&state,0,sizeof(SongState));
}

unsigned int SongManager_Record_Header(void* header_data, void* response_buffer){
    LoadRecHeader(header_data,&rec_file);
    memset(response_buffer,0x00,4);
    memset(response_buffer,2,1);
    return 4;
}
unsigned int SongManager_Record_Body(void* body_data, void* response_buffer){
    LoadRecData(body_data, &rec_file);
    memset(response_buffer,0x00,4);
    memset(response_buffer,2,1);
    return 4;
}

unsigned int SongManager_SongMode_2(void* body_data, void* response_buffer){    
    memset(response_buffer,0x00,4);
    memset(response_buffer,2,1);
    return 4;    
}

unsigned int SongManager_Init(void* setting_data, void* response_buffer){
    printf("SongManager_Init\n");
    SongManager_StopSong();
    ResetStateManagement();
    // Copy our Song Settings Data
    memcpy(&settings,setting_data,sizeof(SongSettings));
    SongJudgeInit(&judge, &settings);

    // Initialize Our Song State Data
    memset(&state,0,sizeof(SongState));  
    memset(&state.player_cursor,0,sizeof(PlayerCursor)*2);
    state.cmd = A27_SONGMODE_MAINGAME_SETTING;
    memcpy(response_buffer,&state,sizeof(SongState));
    return sizeof(SongState);    
}

unsigned int SongManager_Reset(void* response_buffer){
    printf("SongManager_Reset\n");
    state.cmd = A27_SONGMODE_MAINGAME_WAITSTART;
    memcpy(response_buffer,&state,sizeof(SongState));
    return sizeof(SongState);
}

void SongManager_Stop(void){
    printf("SongManager_Stop\n");
    SongManager_StopSong();
    while(SongTimer_IsRunning() || EventTimer_IsRunning() || ScrollTimer_IsRunning() || InputStateTimer_IsRunning()){}   
}

unsigned int SongManager_Start(void* response_buffer){
    printf("SongManager_Start\n");
    PrintSongSetting(&settings);
    for(int i=0; i<2; i++){
        if(settings.player_enable[i]){
            state.player_isplaying[i] = 1;
            state.current_beat[i] = -1;
            state.player_life[i] = judge.player[i].lifebar;
        }
    }
    // Get Name of Event File
    char path_to_recfile[1024] = "./songdata/";
    unsigned int current_chart_id = (settings.player_chartid[0] > settings.player_chartid[1]) ? settings.player_chartid[0] : settings.player_chartid[1];
    // We've already loaded the recorded data if we're in playback mode.
    if(settings.key_record_mode != RECORD_PLAYBACK_MODE){
        GenerateEventFilename(settings.song_mode,current_chart_id,path_to_recfile);
        // Get Song Info from Event File
        LoadRecfile(path_to_recfile,&rec_file);          
    }
    // Load Event File into Song Event Structure
    ParseRecHeader(&rec_file,&event);   
    event.p1_num_cursor_events = ParseCursorEvents((PRecFileLane*)&rec_file.p1_events,(PCursorEvent*)&event.p1_event,event.tempo,settings.player_mod[0].speed);
    event.p2_num_cursor_events = ParseCursorEvents((PRecFileLane*)&rec_file.p2_events,(PCursorEvent*)&event.p2_event,event.tempo,settings.player_mod[1].speed);
    event.num_sound_events = ParseSoundEvents((PRecFileLane*)&rec_file.sound_events,(PSoundEvent*)&event.sound_event,event.tempo);
    GenerateMeasureCursors(&event,settings.player_mod[0].speed,settings.player_mod[1].speed);
    printf("Loaded %d P1 Cursors, %d P2 Cursors, %d Sound Events\n",event.p1_num_cursor_events,event.p2_num_cursor_events,event.num_sound_events);
    SetPlayerVelocity(&event,settings.player_mod[0].speed,settings.player_mod[1].speed);
    
    state.cmd = A27_SONGMODE_MAINGAME_PROCESS;    
    memcpy(response_buffer,&state,sizeof(SongState));
    
    // Start the Timers
    SongManager_StartSong();
    EventTimer_Start(&settings,&state,&event);
    ScrollTimer_Start(&settings,&state,&event);
    InputStateTimer_Start(&settings,&state,&event); 
    SongTimer_Start(&settings,&state,&event);


    // We'll block on waiting for our threads to start.
    while(!SongTimer_IsRunning() || !EventTimer_IsRunning() || !ScrollTimer_IsRunning() || !InputStateTimer_IsRunning()){}
    return sizeof(SongState);    
}

unsigned short last_beat = 0;
unsigned int SongManager_Update(void* response_buffer){
    state.cmd = A27_SONGMODE_MAINGAME_PROCESS;
    
    unsigned short current_beat = SongTimer_GetCurrentBeat(event.ms_per_ebeat);

    if(current_beat != last_beat){
        InputStateTimer_Update();
        Update_Judgement(&settings, &event, &state, &judge);        
    }
    long long song_elapsed = SongTimer_GetSongElapsed();
    for(int i=0; i < event.num_sound_events; i++){
        PSoundEvent ce = &event.sound_event[i];                
        if(ce->spawn_ms > 0 && ce->spawn_ms <= song_elapsed){            
            EventTimer_AddToSoundEvents(ce->event_value);
            ce->spawn_ms = -1;
        }
    }
    
    memcpy(response_buffer,&state,sizeof(SongState));    
    // Reset Sound Index Values
    for(int i=0;i<32;i++){
        state.sound_index[i] = 0;
    }
    last_beat = current_beat;
    memset(&state.player_cursor_hit_animation[0],0x00,sizeof(PlayerAnimation));
    memset(&state.player_judge_graphic[0],0x00,sizeof(PlayerAnimation));
    memset(&state.player_cursor_hit_animation[1],0x00,sizeof(PlayerAnimation));
    memset(&state.player_judge_graphic[1],0x00,sizeof(PlayerAnimation));    
    return sizeof(SongState);    
}

unsigned int SongManager_GetResult(void* response_buffer){
    SongManager_Stop();
    SongResult res = {0};
    res.cmd = A27_SONGMODE_RESULT;
    GetSongResult(&settings,&judge,&res, event.total_notes);
    memcpy(response_buffer,&res,sizeof(SongResult));
    return sizeof(SongResult);
}

