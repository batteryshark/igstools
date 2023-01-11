#include <string.h>
#include <pthread.h>
#include <stdio.h>

#include "../../utils/utils.h"

#include "song_settings.h"
#include "song_state.h"
#include "song_event.h"
#include "song_manager.h"
#include "song_cursor.h"


static unsigned char thread_running = 1; //TODO UNDO THIS
unsigned char JudgeTimer_IsRunning(void){return thread_running;}

void JudgeTimer_Start(PSongSettings song_settings,PSongState state, PSongEvent event){
    
}
void JudgeTimer_Stop(void){
    
}

    // Get the current hit state of our tracks.
/*    
    for(int i=0;i<2;i++){        
         
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
    */
