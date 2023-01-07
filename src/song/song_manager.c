#include <stdio.h>
#include <string.h>

#include "../keyio.h"

#include "song_defs.h"

#include "song_recfile.h"
#include "song_event.h"
#include "song_settings.h"
#include "song_judge.h"
#include "song_state.h"
#include "song_result.h"
#include "song_cursor.h"
#include "../a27/a27.h"
#include "../pm_funcs.h"


#include "song_manager.h"

SongSettings settings;
SongState state;
SongJudge judge;
RecFile rec_file;
SongEvent event;


void ResetSoundEvents(void){
    for(int i=0;i<32;i++){
            state.sound_index[i] = 0;
    }
}

void AddToSoundEvents(unsigned short event_value){
    for(int i=16;i<32;i++){
        if(!state.sound_index[i]){
            state.sound_index[i] = event_value;   
        }
    }    
}

unsigned short GetKeySoundMapping(unsigned char track_index){
    switch(track_index){
        case LANE_BLUE:
            return KEYSOUND_BLUE;
        case LANE_DRUM:
            return KEYSOUND_DRUM;
        case LANE_2DRUM:
            return KEYSOUND_2DRUM;
        case LANE_RIM:
            return KEYSOUND_RIM;
        case LANE_2RIM:
            return KEYSOUND_2RIM;
        case LANE_RED:
            return KEYSOUND_RED;
        default:
            return KEYSOUND_OFF;
    }
}

unsigned int SongManager_Init(void* setting_data, void* response_buffer){
    printf("SongManager_Init\n");
    // Copy our Song Settings Data
    memcpy(&settings,setting_data,sizeof(SongSettings));
    SongJudgeInit(&judge, &settings);
    
    // Initialize Our Song State Data
    memset(&state,0,sizeof(SongState));    
    state.cmd = A27_SONGMODE_MAINGAME_SETTING;
    memcpy(response_buffer,&state,sizeof(SongState));
    return sizeof(SongState);    
}

unsigned int SongManager_Reset(void* response_buffer){
    printf("SongManager_Reset\n");
    state.cmd = A27_SONGMODE_MAINGAME_WAITSTART;
    for(int i=0; i<2; i++){
        if(settings.player_enable[i]){
            state.player_isplaying[i] = 1;
            state.current_beat[i] = -1;
            state.player_life[i] = judge.player[i].lifebar;
        }
    }
    memcpy(response_buffer,&state,sizeof(SongState));
    return sizeof(SongState);
}

void SongManager_Stop(void){
    printf("SongManager_Stop\n");
 // TODO: Stop the Timers, Scrolling, All that shit.   
    //StopTimer();
}


unsigned int SongManager_Start(void* response_buffer){
    printf("SongManager_Start\n");
    PrintSongSetting(&settings);
    
    // Get Name of Event File
    char path_to_recfile[1024] = "./songdata/";
    unsigned int current_chart_id = (settings.player_chartid[0] > settings.player_chartid[1]) ? settings.player_chartid[0] : settings.player_chartid[1];
    GenerateEventFilename(settings.game_mode,current_chart_id,path_to_recfile);
    // Get Song Info from Event File
    LoadRecfile(path_to_recfile,&rec_file);
    // Load Event File into Song Event Structure
    ParseRecHeader(&rec_file,&event);    
    event.p1_num_cursor_events = ParseCursorEvents(rec_file.p1_events,event.p1_event,event.tempo,settings.player_mod[0].speed);
    event.p2_num_cursor_events = ParseCursorEvents(rec_file.p2_events,event.p2_event,event.tempo,settings.player_mod[1].speed);
    event.num_sound_events = ParseSoundEvents(rec_file.sound_events,event.sound_event,event.tempo);
    
    // TODO: Start the Timers, Scrolling, All that shit.    
    state.cmd = A27_SONGMODE_MAINGAME_PROCESS;    
    memcpy(response_buffer,&state,sizeof(SongState));
    return sizeof(SongState);    
}

unsigned int SongManager_Update(void* response_buffer){
    state.cmd = A27_SONGMODE_MAINGAME_PROCESS;
    
    // Get the current beat from the timer.
    short current_beat = GetCurrentBeat();
        
    // Get the current hit state of our tracks.
    PIOTrackStates track_state = GetIOTrackStates();
    
    
    for(int i=0;i<2;i++){        
        state.current_beat[i] = current_beat;     
        // Skip if we aren't using this player.
        if(!state.player_isplaying[i]){continue;}
        
        memset(&state.player_cursor_hit_animation[i],0x00,sizeof(PlayerAnimation));
        memset(&state.player_judge_graphic[i],0x00,sizeof(PlayerAnimation));
        memset(&state.player_track_hit_animation[i],0x00,sizeof(PlayerAnimation));
        
        for(int j=0;j<6;j++){
            state.player_track_hit_animation[i].track[j] = track_state->player[i].track[j];
            if(track_state->player[i].track[j]){
                state.sound_index[(i*8) + j] = GetKeySoundMapping(j);
            }else{
                state.sound_index[(i*8) + j] = KEYSOUND_OFF;
            }
        }
        state.player_track_hit_animation[i].track[LANE_BLUE] = track_state->player[i].track[LANE_BLUE];
        state.player_track_hit_animation[i].track[LANE_DRUM] = track_state->player[i].track[LANE_DRUM];
        state.player_track_hit_animation[i].track[LANE_2DRUM] = track_state->player[i].track[LANE_2DRUM];
        state.player_track_hit_animation[i].track[LANE_RIM] = track_state->player[i].track[LANE_RIM];
        state.player_track_hit_animation[i].track[LANE_2RIM] = track_state->player[i].track[LANE_2RIM];
        state.player_track_hit_animation[i].track[LANE_RED] = track_state->player[i].track[LANE_RED];
        
        // Call the keysounds you need 
        
        
        for(int j=0;j<PLAYER_CURSOR_MAX_ACTIVE;j++){
            // We don't care about cursors that aren't active and measure bars.
            if(IsActiveCursor(state.player_cursor[i].cursor[j]) && !IsMeasureBarCursor(state.player_cursor[i].cursor[j])){
                unsigned char cursor_track = GetCursorTrack(state.player_cursor[i].cursor[j]);
                
                if(IsFeverCursor(state.player_cursor[i].cursor[j])){                    
                    state.player_judge_graphic[i].track[cursor_track] = FeverJudge(&judge,state.player_cursor[i].cursor[j].y_pos, state.player_cursor[i].cursor[j].fever_offset,GetFeverCombo(state.player_cursor[i].cursor[j]), track_state->player[i].track[cursor_track], i, settings.player_autoplay[i], event.total_notes);
                    
                    if(state.player_judge_graphic[i].track[cursor_track]){
                        // If this fever is done, we'll remove the fake result we gave and hide the cursor.
                        if(state.player_judge_graphic[i].track[cursor_track] == ANI_JUDGE_FEVER_OVER){
                            state.player_judge_graphic[i].track[cursor_track] = 0;
                            HideCursor(state.player_cursor[i].cursor[j]);
                        // If this fever is still going, we'll remove the fake judge result and keep the track hit.
                        }else if(state.player_judge_graphic[i].track[cursor_track] == ANI_JUDGE_FEVER_HIT){
                            state.player_judge_graphic[i].track[cursor_track] = 0;
                            state.player_cursor_hit_animation[i].track[cursor_track] = 1;
                            // We're adding a duplicate set of 'track hit' animation cues on the hits because of autoplay.
                            state.player_track_hit_animation[i].track[cursor_track] = 1;
                       }else if(state.player_judge_graphic[i].track[cursor_track] == ANI_JUDGE_BRAVO){                        
                           state.player_cursor_hit_animation[i].track[cursor_track] = 1;  
                           state.player_track_hit_animation[i].track[cursor_track] = 1;
                       }
                    }                    
                }else{
                    state.player_judge_graphic[i].track[cursor_track] = CursorJudge(&judge,state.player_cursor[i].cursor[j].y_pos,cursor_track, track_state->player[i].track[cursor_track], i, settings.player_autoplay[i], event.total_notes);
                    if(state.player_judge_graphic[i].track[cursor_track]){
                        state.player_cursor_hit_animation[i].track[cursor_track] = 1;
                        state.player_track_hit_animation[i].track[cursor_track] = 1;
                        HideCursor(state.player_cursor[i].cursor[j]);
                    }
                }                
            }
        }
        
        // Set State From Judge        
        state.player_combo[i] = judge.player[i].hit_combo;
        state.player_fever_beat[i] = judge.player[i].fever;
        state.player_fever_combo[i] = judge.player[i].current_fever_combo;
        state.player_life[i] = judge.player[i].lifebar;
        state.player_score[i] = judge.player[i].score;
        state.player_score_copy[i] = judge.player[i].score;
    }
    
    // Add any SoundIndex Audio Cues 
    for(int i = 0; i < event.num_sound_events; i++){
        if(event.sound_event[i].spawn_beat > 0 && current_beat >= event.sound_event[i].spawn_beat){
            AddToSoundEvents(event.sound_event[i].event_value);
            event.sound_event[i].spawn_beat = -1;
        }
    }
    memcpy(response_buffer,&state,sizeof(SongState));    
    ResetSoundEvents();
    return sizeof(SongState);    
}


unsigned int SongManager_GetResult(void* response_buffer){
    SongResult res = {0};
    res.cmd = A27_SONGMODE_RESULT;
    GetSongResult(&settings,&judge,&res, event.total_notes);
    memcpy(response_buffer,&res,sizeof(SongResult));
    return sizeof(SongResult);
}
