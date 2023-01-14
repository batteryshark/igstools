#include <stdio.h>
#include <string.h>


#include "song_settings.h"
#include "song_event.h"
#include "song_state.h"

#include "song_manager.h"
#include "song_judge.h"




unsigned char score_multiplier[5] = {10,5,4,2,1};

struct JudgeRange{
    unsigned short deviation;
    unsigned char rating;
};

static struct JudgeRange judge_ranges[] ={
    {0,JV_JUDGE_GREAT},
    {0,JV_JUDGE_COOL},
    {0,JV_JUDGE_NICE},
    {0,JV_JUDGE_POOR},
    {0,JV_JUDGE_MISS}
};

unsigned char GetPositionJudgement(short y_offset) {
    // If we're short of even the "Poor" judgement zone, we don't care.
    // Note: This will also prevent the "miss" in our ranges from firing on the min deviation.
    if(y_offset < JUDGE_CENTER - judge_ranges[1].deviation){return JV_JUDGE_NONE;}
    
    
    for (int i = 0; i < 5; i++) {
        int deviation = judge_ranges[i].deviation;
        if (y_offset < JUDGE_CENTER + deviation && y_offset > JUDGE_CENTER - deviation) {
            return judge_ranges[i].rating;
        }        
    }
    if(y_offset > JUDGE_CENTER + judge_ranges[4].deviation){
        return JV_JUDGE_MISS;
    }
    return JV_JUDGE_GREAT;
}

void SongJudgeInit(PSongJudge judge, PSongSettings song_settings){
    memset(judge,0x00,sizeof(SongJudge));
    
    judge->player[0].lifebar = LIFEBAR_START;
    judge->player[1].lifebar = LIFEBAR_START;
    
    // Set up Judgement Offsets
    judge_ranges[0].deviation = song_settings->judge.great;
    judge_ranges[1].deviation = song_settings->judge.cool;
    judge_ranges[2].deviation = song_settings->judge.nice;
    judge_ranges[3].deviation = song_settings->judge.poor;
    judge_ranges[4].deviation = song_settings->judge.poor + 20;
    
    judge->settings.offsets.great = song_settings->judge.great;
    judge->settings.offsets.cool = song_settings->judge.cool;    
    judge->settings.offsets.nice = song_settings->judge.nice;    
    judge->settings.offsets.poor = song_settings->judge.poor;
    judge->settings.offsets.miss = song_settings->judge.poor + 20;
    

    for(int i=0;i<2;i++){
        judge->settings.lifebar_rate[i].fever = song_settings->level_rate[i].fever;
        judge->settings.lifebar_rate[i].great = song_settings->level_rate[i].great;
        judge->settings.lifebar_rate[i].cool = song_settings->level_rate[i].cool;
        judge->settings.lifebar_rate[i].nice = song_settings->level_rate[i].nice;
        judge->settings.lifebar_rate[i].poor = song_settings->level_rate[i].poor;
        judge->settings.lifebar_rate[i].miss = song_settings->level_rate[i].miss;
    }
    
}


// This is executed on every non-hitless cursor to determine if the player hit the cursor and update their state accordingly.
unsigned char CursorJudge(short cursor_y, unsigned char player_autoplay, unsigned char is_fever){

    // If it's outside of the judgement zone, we don't care.
    unsigned char judgement = GetPositionJudgement(cursor_y);
    
    // To support autoplay, we'll also return until we're in the "GREAT" range.
    if(player_autoplay && judgement != JV_JUDGE_GREAT && !is_fever){        
        return JV_JUDGE_NONE;
    }
    return judgement;
}

unsigned short GetMaxFeverBeat(short offset, unsigned char player_velocity){
    float max_fever_beats = (float)offset * -1;
    max_fever_beats = max_fever_beats / player_velocity;
    return (unsigned short)max_fever_beats;
}

void Update_Judgement(PSongSettings settings, PSongEvent event, PSongState state, PSongJudge judge){
    PIOTrackStates track_states = InputStateTimer_GetTrackState();
    PCursorTimestamps ts = EventTimer_GetCursorTS();
    for(int i=0;i<2;i++){     
        // Skip if we aren't using this player.
        if(!state->player_isplaying[i]){continue;}
        for(int j=0;j<PLAYER_CURSOR_MAX_ACTIVE;j++){
            // We don't care about cursors that aren't active.
            if(!IsActivePCursor(ts,i,j)){continue;}
            // We also don't care about hidden cursors and measure bars.
            if(IsMeasureBarCursor(state->player_cursor[i].cursor[j]) || IsHiddenCursor(state->player_cursor[i].cursor[j])){continue;}
            // First of all - get the cursor track.
            unsigned char cursor_track = GetCursorTrack(state->player_cursor[i].cursor[j]);
            
            // If it's a fever cursor, we have a bunch of fever checking logic. We have an "in_fever" state we have to watch. We also have to know when to exit the fever etc.
            // How about for now we just skip it and continue;
            if(IsFeverCursor(state->player_cursor[i].cursor[j])){ 
                unsigned char in_fever = state->player_fever_beat[i];
                unsigned char judgement = CursorJudge(state->player_cursor[i].cursor[j].y_pos + state->player_cursor[i].cursor[j].fever_offset,settings->player_autoplay[i],1);
                // If y_pos < JUDGE_CENTER, there's nothing for us to do yet.
                if(state->player_cursor[i].cursor[j].y_pos < JUDGE_CENTER){continue;}

                // We'll set the track state if we're on autoplay.
                if(settings->player_autoplay[i]){
                    track_states->player[i].track[cursor_track] = 1;
                }
                unsigned char player_hit_cursor = track_states->player[i].track[cursor_track];
                
                // If we're done our fever, it's time to figure out if we missed or not.
                if(in_fever && judgement == JV_JUDGE_MISS){
                    judgement = JV_JUDGE_FEVER_END; 
                    // In this case, a "Missed" FEVER involves not hitting the minimum combo for notes.
                    if(state->player_fever_beat[i] > state->player_fever_combo[i]){
                        unsigned short hit_amount = state->player_fever_beat[i] - state->player_fever_combo[i];
                        state->player_judge_graphic[i].track[cursor_track] = ANI_JUDGE_MISS;
                        // Record Max Combo if necessary and reset Hit Combo
                        if(judge->player[i].max_combo < judge->player[i].hit_combo){
                            judge->player[i].hit_combo = judge->player[i].max_combo;   
                        }
                        judge->player[i].hit_combo = 0;
                        // Increment Our Miss Count - Note, we might be super cruel and take the difference here.
                        // Actually, yeah - fuck it, if you miss like 8 notes on a fever you'll get 8 misses... life is unfair.
                        judge->player[i].miss+=hit_amount;
                        
                        // Update Our Current Lifebar 
                        judge->player[i].lifebar += (judge->settings.lifebar_rate[i].miss * hit_amount);
                        
                        // Snap to 0 if Lifebar is < 0 and if we hit zero, mark it for results later.
                        if(judge->player[i].lifebar < LIFEBAR_MIN){
                            judge->player[i].lifebar = LIFEBAR_MIN;
                        }
                        if(judge->player[i].lifebar == LIFEBAR_MIN){
                            judge->player[i].lifebar_hit_zero = 1;
                        }
                        state->player_judge_graphic[i].track[cursor_track] = ANI_JUDGE_MISS;
                    }
                    state->player_fever_beat[i] = 0;
                    judge->player[i].total_fever_hits+=state->player_fever_combo[i];
                    state->player_fever_combo[i] = 0;
                    HideCursor(state->player_cursor[i].cursor[j]);
                }else{                    
                    if(player_hit_cursor){
                        // This is now assuming we're in range.
                        state->player_fever_beat[i] = (state->player_cursor[i].cursor[j].fever_flags >> 2);                        
                        float lifebar_rate = judge->settings.lifebar_rate[i].fever;                        
                        unsigned short fever_max_amount = GetMaxFeverBeat(state->player_cursor[i].cursor[j].fever_offset,event->player_velocity[i]);
                        
                        if(state->player_fever_combo[i] < fever_max_amount){
                            if(state->player_fever_combo[i] < state->player_fever_beat[i]){
                                judge->player[i].fever++;  
                                judge->player[i].hit_combo++;                                
                            }
                            state->player_fever_combo[i]++;                                                        
                            // Update Score
                            judge->player[i].score += (score_multiplier[JUDGE_FEVER] * judge->player[i].hit_combo);
                            if(event->total_notes == judge->player[i].hit_combo){
                                state->player_judge_graphic[i].track[cursor_track] = ANI_JUDGE_BRAVO;
                            }     
                            state->player_cursor_hit_animation[i].track[cursor_track] = 1;
                            // Update Lifebar
                            judge->player[i].lifebar += lifebar_rate ;
                            // Snap Lifebar to Max if Necessary.
                            if(judge->player[i].lifebar > LIFEBAR_MAX){
                                judge->player[i].lifebar = LIFEBAR_MAX;
                            }                            
                        }                    
                    }
                }

            }else{
                
            // We'll get the judgement of the cursor at this point.
            unsigned char judgement = CursorJudge(state->player_cursor[i].cursor[j].y_pos,settings->player_autoplay[i],0);
            if(judgement == JV_JUDGE_NONE){
                continue;
            }
            // We'll set the track state if we're on autoplay.
            if(judgement == JV_JUDGE_GREAT && settings->player_autoplay[i]){
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
                state->player_judge_graphic[i].track[cursor_track] = ANI_JUDGE_MISS;
                // Record Max Combo if necessary and reset Hit Combo
                if(judge->player[i].max_combo < judge->player[i].hit_combo){
                    judge->player[i].hit_combo = judge->player[i].max_combo;   
                }
                judge->player[i].hit_combo = 0;
                // Increment Our Miss Count
                judge->player[i].miss+=hit_inc;
                
                // Update Our Current Lifebar 
                judge->player[i].lifebar += (judge->settings.lifebar_rate[i].miss * hit_inc);
                
                // Snap to 0 if Lifebar is < 0 and if we hit zero, mark it for results later.
                if(judge->player[i].lifebar < LIFEBAR_MIN){
                    judge->player[i].lifebar = LIFEBAR_MIN;
                }
                if(judge->player[i].lifebar == LIFEBAR_MIN){
                    judge->player[i].lifebar_hit_zero = 1;
                }
                HideCursor(state->player_cursor[i].cursor[j]);
            }else if(player_hit_cursor){
                float lifebar_rate = 0;
                unsigned int score_mult = 0;                
                if(judgement == JV_JUDGE_GREAT){
                    judge->player[i].great+=hit_inc;
                    lifebar_rate = judge->settings.lifebar_rate[i].great;
                    score_mult = score_multiplier[JUDGE_GREAT];
                    state->player_judge_graphic[i].track[cursor_track] = ANI_JUDGE_GREAT;
                    HideCursor(state->player_cursor[i].cursor[j]);
                }else if(judgement == JV_JUDGE_COOL){
                    judge->player[i].cool+=hit_inc;
                    lifebar_rate = judge->settings.lifebar_rate[i].cool;
                    score_mult = score_multiplier[JUDGE_COOL];
                    state->player_judge_graphic[i].track[cursor_track] = ANI_JUDGE_COOL;
                    HideCursor(state->player_cursor[i].cursor[j]);
                }else if(judgement == JV_JUDGE_NICE){
                    judge->player[i].nice+=hit_inc;
                    lifebar_rate = judge->settings.lifebar_rate[i].nice;
                    score_mult = score_multiplier[JUDGE_NICE];
                    state->player_judge_graphic[i].track[cursor_track] = ANI_JUDGE_NICE;
                    HideCursor(state->player_cursor[i].cursor[j]);
                }else if(judgement == JV_JUDGE_POOR){
                    judge->player[i].poor+=hit_inc;
                    lifebar_rate = judge->settings.lifebar_rate[i].poor;
                    score_mult = score_multiplier[JUDGE_POOR];
                    state->player_judge_graphic[i].track[cursor_track] = ANI_JUDGE_POOR;
                    HideCursor(state->player_cursor[i].cursor[j]);
                }   

                // We don't update a score in record mode.
                if(settings->key_record_mode != RECORD_SONG_MODE){

                    // Update Hit Combo
                    judge->player[i].hit_combo+=hit_inc;
                    
                    // Update Score
                    judge->player[i].score += (score_mult * judge->player[i].hit_combo);

                    // Update Lifebar
                    judge->player[i].lifebar += (lifebar_rate * hit_inc);
                    // Snap Lifebar to Max if Necessary.
                    if(judge->player[i].lifebar > LIFEBAR_MAX){
                        judge->player[i].lifebar = LIFEBAR_MAX;
                    }
                        
                    // If we full combo'd the song, we'll send a different animation back.
                    if(event->total_notes == judge->player[i].hit_combo){
                        state->player_judge_graphic[i].track[cursor_track] = ANI_JUDGE_BRAVO;
                    }
                
                }
                state->player_cursor_hit_animation[i].track[cursor_track] = 1;
                
            }
            
        }
            }

            

        // Set State From Judge        
        state->player_combo[i] = judge->player[i].hit_combo;        
        state->player_life[i] = judge->player[i].lifebar;
        state->player_score[i] = judge->player[i].score;
        state->player_score_copy[i] = judge->player[i].score;           
    }
}
