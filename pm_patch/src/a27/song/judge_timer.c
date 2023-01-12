#include <string.h>
#include <pthread.h>
#include <stdio.h>

#include "../../utils/utils.h"

#include "song_settings.h"
#include "song_state.h"
#include "song_event.h"
#include "song_manager.h"
#include "song_cursor.h"
#include "song_judge.h"

typedef struct _THREAD_PARAMS{
    PSongSettings settings;
    PSongEvent event;
    PSongState state;
    PSongJudge judge;
}ThreadParams,*PThreadParams;
static pthread_t judgetimer_hthread;
static ThreadParams tp;
static unsigned char thread_running = 0;


unsigned char JudgeTimer_IsRunning(void){return thread_running;}

unsigned char score_mult_table[5] = {10,5,4,2,1};

void JudgeTimer_Update(void){
    PIOTrackStates track_states = InputStateTimer_GetTrackState();
    PCursorTimestamps ts = EventTimer_GetCursorTS();
    for(int i=0;i<2;i++){     
        // Skip if we aren't using this player.
        if(!tp.state->player_isplaying[i]){continue;}
        for(int j=0;j<PLAYER_CURSOR_MAX_ACTIVE;j++){
            // We don't care about cursors that aren't active.
            if(!IsActivePCursor(ts,i,j)){continue;}
            // We also don't care about hidden cursors and measure bars.
            if(IsMeasureBarCursor(tp.state->player_cursor[i].cursor[j]) || IsHiddenCursor(tp.state->player_cursor[i].cursor[j])){continue;}
            // First of all - get the cursor track.
            unsigned char cursor_track = GetCursorTrack(tp.state->player_cursor[i].cursor[j]);
            
            // If it's a fever cursor, we have a bunch of fever checking logic. We have an "in_fever" state we have to watch. We also have to know when to exit the fever etc.
            // How about for now we just skip it and continue;
            if(IsFeverCursor(tp.state->player_cursor[i].cursor[j])){ 
                // TODO: Re-Add This Logic
                continue;
            }
            
            // We'll get the judgement of the cursor at this point.
            unsigned char judgement = CursorJudge(tp.state->player_cursor[i].cursor[j].y_pos,tp.settings->player_autoplay[i]);
            if(judgement == JV_JUDGE_NONE){
                continue;
            }
            // We'll set the track state if we're on autoplay.
            if(judgement == JV_JUDGE_GREAT && tp.settings->player_autoplay[i]){
                track_states->player[i].track[cursor_track] = 1;
            }
            unsigned char player_hit_cursor = track_states->player[i].track[cursor_track];            
            // To account for double-hit cursors, we have to increment the beat count.
            int hit_inc = 1;
            if(cursor_track == 2 || cursor_track == 4){
                hit_inc++;
            }
            // Handle Miss (or "Lost")
            if(judgement == JV_JUDGE_MISS){
                printf("Judge Miss\n");
                tp.state->player_judge_graphic[i].track[cursor_track] = ANI_JUDGE_MISS;
                // Record Max Combo if necessary and reset Hit Combo
                if(tp.judge->player[i].max_combo < tp.judge->player[i].hit_combo){
                    tp.judge->player[i].hit_combo = tp.judge->player[i].max_combo;   
                }
                tp.judge->player[i].hit_combo = 0;
                // Increment Our Miss Count
                tp.judge->player[i].miss+=hit_inc;
                
                // Update Our Current Lifebar 
                tp.judge->player[i].lifebar += (tp.judge->settings.lifebar_rate[i].miss * hit_inc);
                
                // Snap to 0 if Lifebar is < 0 and if we hit zero, mark it for results later.
                if(tp.judge->player[i].lifebar < LIFEBAR_MIN){
                    tp.judge->player[i].lifebar = LIFEBAR_MIN;
                }
                if(tp.judge->player[i].lifebar == LIFEBAR_MIN){
                    tp.judge->player[i].lifebar_hit_zero = 1;
                }
                HideCursor(tp.state->player_cursor[i].cursor[j]);
            }else if(player_hit_cursor){
                float lifebar_rate = 0;
                unsigned int score_mult = 0;                
                if(judgement == JV_JUDGE_GREAT){
                    tp.judge->player[i].great+=hit_inc;
                    lifebar_rate = tp.judge->settings.lifebar_rate[i].great;
                    score_mult = score_mult_table[JUDGE_GREAT];
                    tp.state->player_judge_graphic[i].track[cursor_track] = ANI_JUDGE_GREAT;
                    HideCursor(tp.state->player_cursor[i].cursor[j]);
                }else if(judgement == JV_JUDGE_COOL){
                    tp.judge->player[i].cool+=hit_inc;
                    lifebar_rate = tp.judge->settings.lifebar_rate[i].cool;
                    score_mult = score_mult_table[JUDGE_COOL];
                    tp.state->player_judge_graphic[i].track[cursor_track] = ANI_JUDGE_COOL;
                    HideCursor(tp.state->player_cursor[i].cursor[j]);
                }else if(judgement == JV_JUDGE_NICE){
                    tp.judge->player[i].nice+=hit_inc;
                    lifebar_rate = tp.judge->settings.lifebar_rate[i].nice;
                    score_mult = score_mult_table[JUDGE_NICE];
                    tp.state->player_judge_graphic[i].track[cursor_track] = ANI_JUDGE_NICE;
                    HideCursor(tp.state->player_cursor[i].cursor[j]);
                }else if(judgement == JV_JUDGE_POOR){
                    tp.judge->player[i].poor+=hit_inc;
                    lifebar_rate = tp.judge->settings.lifebar_rate[i].poor;
                    score_mult = score_mult_table[JUDGE_POOR];
                    tp.state->player_judge_graphic[i].track[cursor_track] = ANI_JUDGE_POOR;
                    HideCursor(tp.state->player_cursor[i].cursor[j]);
                }   
                // Update Score
                tp.judge->player[i].score += (score_mult * tp.judge->player[i].hit_combo);

                // Update Hit Combo
                tp.judge->player[i].hit_combo+=hit_inc;

                // Update Lifebar
                tp.judge->player[i].lifebar += (lifebar_rate * hit_inc);
                // Snap Lifebar to Max if Necessary.
                if(tp.judge->player[i].lifebar > LIFEBAR_MAX){
                    tp.judge->player[i].lifebar = LIFEBAR_MAX;
                }
                    
                // If we full combo'd the song, we'll send a different animation back.
                if(tp.event->total_notes == tp.judge->player[i].hit_combo){
                    tp.state->player_judge_graphic[i].track[cursor_track] = ANI_JUDGE_BRAVO;
                }
                tp.state->player_cursor_hit_animation[i].track[cursor_track] = 1;
                
            }
            
        }
        // Set State From Judge        
        tp.state->player_combo[i] = tp.judge->player[i].hit_combo;        
        tp.state->player_fever_combo[i] = tp.judge->player[i].current_fever_combo;
        tp.state->player_life[i] = tp.judge->player[i].lifebar;
        tp.state->player_score[i] = tp.judge->player[i].score;
        tp.state->player_score_copy[i] = tp.judge->player[i].score;           
    }
}

static void *judge_timer_thread(void* arg){
    PIOTrackStates track_states = InputStateTimer_GetTrackState();
    PCursorTimestamps ts = EventTimer_GetCursorTS();
    thread_running = 1;
    while(SongManager_InSong()){
        continue;
        for(int i=0;i<2;i++){     
            // Skip if we aren't using this player.
            if(!tp.state->player_isplaying[i]){continue;}
            for(int j=0;j<PLAYER_CURSOR_MAX_ACTIVE;j++){
                // We don't care about cursors that aren't active.
                if(!IsActivePCursor(ts,i,j)){continue;}
                // We also don't care about hidden cursors and measure bars.
                if(IsMeasureBarCursor(tp.state->player_cursor[i].cursor[j]) || IsHiddenCursor(tp.state->player_cursor[i].cursor[j])){continue;}
                // First of all - get the cursor track.
                unsigned char cursor_track = GetCursorTrack(tp.state->player_cursor[i].cursor[j]);
                
                // If it's a fever cursor, we have a bunch of fever checking logic. We have an "in_fever" state we have to watch. We also have to know when to exit the fever etc.
                // How about for now we just skip it and continue;
                if(IsFeverCursor(tp.state->player_cursor[i].cursor[j])){ 
                 // TODO: Re-Add This Logic
                    continue;
                }
                
                // We'll get the judge graphic based on our judgement now.
                //tp.state->player_judge_graphic[i].track[cursor_track] = CursorJudge(tp.judge,tp.state->player_cursor[i].cursor[j].y_pos,cursor_track,track_states->player[i].track[cursor_track], i, tp.settings->player_autoplay[i], tp.event->total_notes);
                
                /*
                else{
                    
                    if(tp.state->player_judge_graphic[i].track[cursor_track]){
                        tp.state->player_cursor_hit_animation[i].track[cursor_track] = 1;
                        tp.state->player_track_hit_animation[i].track[cursor_track] = 1;
                        if(tp.settings->player_autoplay[i]){
                            tp.state->sound_index[(i*8) + cursor_track] = InputStateTimer_GetKeySoundMapping(cursor_track);   
                        }
                        tp.state->player_judge_graphic[i].track[cursor_track] = ANI_JUDGE_GREAT;
                        
                        // Handle Miss Stuff
                        if(tp.state->player_judge_graphic[i].track[cursor_track] == ANI_JUDGE_MISS){
                            tp.state->player_cursor_hit_animation[i].track[cursor_track] = 0;
                            tp.state->player_track_hit_animation[i].track[cursor_track] = 0;
                        }
                        HideCursor(tp.state->player_cursor[i].cursor[j]);
                    }                    
                } */  
                
            }
         
        }
        
        
        // We'll sleep to only do this every 1ms for now.
        SleepMS(1);
    }
    thread_running = 0;
}

/*
 if(IsFeverCursor(tp.state->player_cursor[i].cursor[j])){ 
                    tp.state->player_fever_beat[i] = GetFeverCombo(tp.state->player_cursor[i].cursor[j]);
                    tp.state->player_judge_graphic[i].track[cursor_track] = FeverJudge(tp.judge,tp.state->player_cursor[i].cursor[j].y_pos, tp.state->player_cursor[i].cursor[j].fever_offset,GetFeverCombo(tp.state->player_cursor[i].cursor[j]), track_states->player[i].track[cursor_track], i, tp.settings->player_autoplay[i], tp.event->total_notes);
                    
                    if(tp.state->player_judge_graphic[i].track[cursor_track]){
                        // If this fever is done, we'll remove the fake result we gave and hide the cursor.
                        if(tp.state->player_judge_graphic[i].track[cursor_track] == ANI_JUDGE_FEVER_OVER){
                            tp.state->player_judge_graphic[i].track[cursor_track] = 0;
                            tp.state->player_fever_beat[i] = 0;
                            HideCursor(tp.state->player_cursor[i].cursor[j]);
                        // If this fever is still going, we'll remove the fake judge result and keep the track hit.
                        }else if(tp.state->player_judge_graphic[i].track[cursor_track] == ANI_JUDGE_FEVER_HIT){
                            tp.state->player_judge_graphic[i].track[cursor_track] = 0;
                            tp.state->player_cursor_hit_animation[i].track[cursor_track] = 1;
                            // We're adding a duplicate set of 'track hit' animation cues on the hits because of autoplay.
                            tp.state->player_track_hit_animation[i].track[cursor_track] = 1;
                       }else if(tp.state->player_judge_graphic[i].track[cursor_track] == ANI_JUDGE_BRAVO){                        
                           tp.state->player_cursor_hit_animation[i].track[cursor_track] = 1;  
                           tp.state->player_track_hit_animation[i].track[cursor_track] = 1;
                       }
                    }                    
                }
 */

void JudgeTimer_Start(PSongSettings song_settings,PSongState state, PSongEvent event, PSongJudge judge){
    if(thread_running){return;}
    tp.event = event;
    tp.settings = song_settings;
    tp.state = state;
    tp.judge = judge;
    pthread_create(&judgetimer_hthread, 0, judge_timer_thread, NULL);
}

