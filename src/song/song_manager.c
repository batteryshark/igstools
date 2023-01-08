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
#include "song_timer.h"
#include "../a27/a27.h"
#include "../pm_funcs.h"


#include "song_manager.h"

SongSettings settings;
SongState state;
SongJudge judge;
RecFile rec_file;
SongEvent event;
IOTrackStates track_state;
IOTrackStates last_track_state;

void ResetSoundEvents(void){
    for(int i=0;i<32;i++){
            state.sound_index[i] = 0;
    }
}


static unsigned int last_last_swst = 0;
static unsigned int last_swst = 0;
void UpdateTrackInput(void){
    
    unsigned int swst = KeyIO_GetSwitches();
    memcpy(&last_track_state,&track_state,sizeof(IOTrackStates));
       
    if(state.player_isplaying[0]){
        track_state.player[0].track[LANE_BLUE] = IO_ISSSET(swst,last_swst,INP_P1_BLUE);
        track_state.player[0].track[LANE_DRUM] = IO_ISSSET(swst,last_swst,INP_P1_DRUM_L) || IO_ISSSET(swst,last_swst,INP_P1_DRUM_R);
        track_state.player[0].track[LANE_2DRUM] = (IO_ISSSET(swst,last_swst,INP_P1_DRUM_L) + IO_ISSSET(swst,last_swst,INP_P1_DRUM_R) + IO_ISSSET(last_swst,last_last_swst,INP_P1_DRUM_L) + IO_ISSSET(last_swst,last_last_swst,INP_P1_DRUM_R) > 1);
        track_state.player[0].track[LANE_RIM] = IO_ISSSET(swst,last_swst,INP_P1_RIM_L) || IO_ISSSET(swst,last_swst,INP_P1_RIM_R);
        track_state.player[0].track[LANE_2RIM] = (IO_ISSSET(swst,last_swst,INP_P1_RIM_L) + IO_ISSSET(swst,last_swst,INP_P1_RIM_R) + IO_ISSSET(last_swst,last_last_swst,INP_P1_RIM_L) + IO_ISSSET(last_swst,last_last_swst,INP_P1_RIM_R) > 1);
        track_state.player[0].track[LANE_RED] = IO_ISSSET(swst,last_swst,INP_P1_RED);
        
        // If we hit both on rim or drum, we have to flip the single lanes.
        if(track_state.player[0].track[LANE_2DRUM]){
            track_state.player[0].track[LANE_DRUM] = 0;
            last_track_state.player[0].track[LANE_2DRUM] = 1;            
        }
        if(track_state.player[0].track[LANE_2RIM]){
            track_state.player[0].track[LANE_RIM] = 0;
            last_track_state.player[0].track[LANE_2RIM] = 1;
        }
    }
    if(state.player_isplaying[1]){
        track_state.player[1].track[LANE_BLUE] = IO_ISSET(swst,INP_P2_BLUE);
        track_state.player[1].track[LANE_DRUM] = IO_ISSET(swst,INP_P2_DRUM_L) || IO_ISSET(swst,INP_P2_DRUM_R);
        track_state.player[1].track[LANE_2DRUM] = (IO_ISSET(swst,INP_P2_DRUM_L) + IO_ISSET(swst,INP_P2_DRUM_R) + IO_ISSET(last_swst,INP_P2_DRUM_L) + IO_ISSET(last_swst,INP_P2_DRUM_R) > 1);
        track_state.player[1].track[LANE_RIM] = IO_ISSET(swst,INP_P2_RIM_L) || IO_ISSET(swst,INP_P2_RIM_R);
        track_state.player[1].track[LANE_2RIM] =  (IO_ISSET(swst,INP_P2_RIM_L) + IO_ISSET(swst,INP_P2_RIM_R) + IO_ISSET(last_swst,INP_P2_RIM_L) + IO_ISSET(last_swst,INP_P2_RIM_R) > 1);
        track_state.player[1].track[LANE_RED] = IO_ISSET(swst,INP_P2_RED);
        
        // If we hit both on rim or drum, we have to flip the single lanes.
        if(track_state.player[1].track[LANE_2DRUM]){
            track_state.player[1].track[LANE_DRUM] = 0;
        }
        if(track_state.player[1].track[LANE_2RIM]){
            track_state.player[1].track[LANE_RIM] = 0;
        }
    }
    last_last_swst = last_swst;
    last_swst = swst; 
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
    memcpy(response_buffer,&state,sizeof(SongState));
    return sizeof(SongState);
}

void SongManager_Stop(void){
    printf("SongManager_Stop\n");
    StopSongThreads();
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
    GenerateEventFilename(settings.song_mode,current_chart_id,path_to_recfile);
    // Get Song Info from Event File
    LoadRecfile(path_to_recfile,&rec_file);
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
    StartSongThreads(&settings,&state, &event, &track_state);
    return sizeof(SongState);    
}

unsigned int SongManager_Update(void* response_buffer){
    state.cmd = A27_SONGMODE_MAINGAME_PROCESS;

    // Get the current beat from the timer.
    short current_beat = GetCurrentBeat();
        
    // Get the current hit state of our tracks.
    
    for(int i=0;i<2;i++){        
        state.current_beat[i] = current_beat;     
        // Skip if we aren't using this player.
        if(!state.player_isplaying[i]){continue;}
        
        memset(&state.player_cursor_hit_animation[i],0x00,sizeof(PlayerAnimation));
        memset(&state.player_judge_graphic[i],0x00,sizeof(PlayerAnimation));
        memset(&state.player_track_hit_animation[i],0x00,sizeof(PlayerAnimation));
        
        // Get Track State
        UpdateTrackInput();
        
        // Call the keysounds you need 
        for(int j=0;j<6;j++){
            if(last_track_state.player[i].track[j]){
                state.sound_index[(i*8) + j] = GetKeySoundMapping(j);
            }else{
                state.sound_index[(i*8) + j] = KEYSOUND_OFF;
            }
        }
        state.player_track_hit_animation[i].track[LANE_BLUE] = last_track_state.player[i].track[LANE_BLUE];
        state.player_track_hit_animation[i].track[LANE_DRUM] = last_track_state.player[i].track[LANE_DRUM];
        state.player_track_hit_animation[i].track[LANE_2DRUM] = last_track_state.player[i].track[LANE_2DRUM];
        state.player_track_hit_animation[i].track[LANE_RIM] = last_track_state.player[i].track[LANE_RIM];
        state.player_track_hit_animation[i].track[LANE_2RIM] = last_track_state.player[i].track[LANE_2RIM];
        state.player_track_hit_animation[i].track[LANE_RED] = last_track_state.player[i].track[LANE_RED];
        
        
        
        
        for(int j=0;j<PLAYER_CURSOR_MAX_ACTIVE;j++){
            // We don't care about cursors that aren't active and measure bars.
            if(!IsHiddenCursor(state.player_cursor[i].cursor[j]) && !IsMeasureBarCursor(state.player_cursor[i].cursor[j])){
                unsigned char cursor_track = GetCursorTrack(state.player_cursor[i].cursor[j]);
                
                if(IsFeverCursor(state.player_cursor[i].cursor[j])){ 
                    state.player_fever_beat[i] = GetFeverCombo(state.player_cursor[i].cursor[j]);
                    state.player_judge_graphic[i].track[cursor_track] = FeverJudge(&judge,state.player_cursor[i].cursor[j].y_pos, state.player_cursor[i].cursor[j].fever_offset,GetFeverCombo(state.player_cursor[i].cursor[j]), last_track_state.player[i].track[cursor_track], i, settings.player_autoplay[i], event.total_notes);
                    
                    if(state.player_judge_graphic[i].track[cursor_track]){
                        // If this fever is done, we'll remove the fake result we gave and hide the cursor.
                        if(state.player_judge_graphic[i].track[cursor_track] == ANI_JUDGE_FEVER_OVER){
                            state.player_judge_graphic[i].track[cursor_track] = 0;
                            state.player_fever_beat[i] = 0;
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
                    state.player_judge_graphic[i].track[cursor_track] = CursorJudge(&judge,state.player_cursor[i].cursor[j].y_pos,cursor_track, last_track_state.player[i].track[cursor_track], i, settings.player_autoplay[i], event.total_notes);
                    if(state.player_judge_graphic[i].track[cursor_track]){
                        state.player_cursor_hit_animation[i].track[cursor_track] = 1;
                        state.player_track_hit_animation[i].track[cursor_track] = 1;
                        if(settings.player_autoplay[i]){
                            state.sound_index[(i*8) + cursor_track] = GetKeySoundMapping(cursor_track);   
                        }
                        state.player_judge_graphic[i].track[cursor_track] = ANI_JUDGE_GREAT;
                        
                        // Handle Miss Stuff
                        if(state.player_judge_graphic[i].track[cursor_track] == ANI_JUDGE_MISS){
                            state.player_cursor_hit_animation[i].track[cursor_track] = 0;
                            state.player_track_hit_animation[i].track[cursor_track] = 0;
                        }
                        HideCursor(state.player_cursor[i].cursor[j]);
                    }                    
                }                
            }
        }
        
        // Set State From Judge        
        state.player_combo[i] = judge.player[i].hit_combo;
        
        state.player_fever_combo[i] = judge.player[i].current_fever_combo;
        state.player_life[i] = judge.player[i].lifebar;
        state.player_score[i] = judge.player[i].score;
        state.player_score_copy[i] = judge.player[i].score;
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
