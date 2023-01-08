#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "song_timer.h"
#include "song_manager.h"
#include "song_utils.h"
#include "a27.h"
#include "song_defs.h"

#include "../keyio.h"

static SongState song_state;
static SongSetting song_setting;
static SongInfo song_info;

static SongParams sparms;

static short judge_min;
static short judge_max;
static short judge_great_min;
static short judge_great_max;
static short judge_cool_min;
static short judge_cool_max;
static short judge_nice_min;
static short judge_nice_max;
static short judge_poor_min;
static short judge_poor_max;

PlayerJudge p1_judge;
PlayerJudge p2_judge;

PlayerHitState p1_last_hitstate;
PlayerHitState p2_last_hitstate;






// --- Interface ---


static unsigned int last_swst;
void SongManager_Update(void* msg){
    struct A27_Read_Message* rmsg = msg;
    
    song_state.cmd = A27_SONGMODE_MAINGAME_PROCESS;

    PlayerHitState p1_hitstate;
    PlayerHitState p2_hitstate;
    unsigned int swst = KeyIO_GetSwitches();
    ClearKeySounds(&song_state);
    short current_beat = GetCurrentBeat();
    // Do All the Player Update Stuff Based on if the Player is Playing
    if(song_state.p1_playing){
        // Update Beat Counter
        song_state.p1_current_beat = current_beat;

        // Update Input State Stuff
        p1_hitstate.blue = IO_ISSSET(swst,last_swst,INP_P1_BLUE);
        p1_hitstate.red = IO_ISSSET(swst,last_swst,INP_P1_RED);
        p1_hitstate.drum_l = IO_ISSSET(swst,last_swst,INP_P1_DRUM_L);
        p1_hitstate.drum_r = IO_ISSSET(swst,last_swst,INP_P1_DRUM_R);
        p1_hitstate.drum = (p1_hitstate.drum_l || p1_hitstate.drum_r);
        p1_hitstate.rim_l = IO_ISSSET(swst,last_swst,INP_P1_RIM_L);
        p1_hitstate.rim_r = IO_ISSSET(swst,last_swst,INP_P1_RIM_R);        
        p1_hitstate.rim = (p1_hitstate.rim_l || p1_hitstate.rim_r);       
        
        
        p1_hitstate.drum_both = ((p1_hitstate.drum_l + p1_hitstate.drum_r + p1_last_hitstate.drum_l + p1_last_hitstate.drum_r) > 1);
        p1_hitstate.rim_both = ((p1_hitstate.rim_l + p1_hitstate.rim_r + p1_last_hitstate.rim_l + p1_last_hitstate.rim_r) > 1);

        // We have to time travel a bit to get the multi hit working.
        if(p1_hitstate.drum_both){
            p1_last_hitstate.drum_both = 1;
            p1_hitstate.drum_both = 0;
        }
            if(p1_hitstate.rim_both){
            p1_last_hitstate.rim_both = 1;   
            p1_hitstate.rim_both = 0;
        }        
        
        // Illuminate Lanes based on Last Input
        song_state.p1_lane_animation[LANE_BLUE] = p1_last_hitstate.blue;
        song_state.p1_lane_animation[LANE_DRUM] = p1_last_hitstate.drum; 
        song_state.p1_lane_animation[LANE_2DRUM] = p1_last_hitstate.drum_both;
        song_state.p1_lane_animation[LANE_RIM] = p1_last_hitstate.rim;
        song_state.p1_lane_animation[LANE_2RIM] = p1_last_hitstate.rim_both;
        song_state.p1_lane_animation[LANE_RED] = p1_last_hitstate.red;
        
        // Assign KeySounds Based on Last Input
        SetKeySound(&song_state,LANE_BLUE,p1_last_hitstate.blue ? KEYSOUND_BLUE:KEYSOUND_OFF);
        SetKeySound(&song_state,LANE_RED,p1_last_hitstate.red ? KEYSOUND_RED:KEYSOUND_OFF);
        SetKeySound(&song_state,LANE_DRUM,p1_last_hitstate.drum ? KEYSOUND_DRUM:KEYSOUND_OFF);
        SetKeySound(&song_state,LANE_RIM,p1_last_hitstate.rim ? KEYSOUND_RIM:KEYSOUND_OFF);

        // ... except for the Multi-Hits because we need current input for those -_-
        if(p1_last_hitstate.drum_both){
            SetKeySound(&song_state,LANE_DRUM,KEYSOUND_OFF);
        }
        if(p1_last_hitstate.rim_both){
            SetKeySound(&song_state,LANE_RIM,KEYSOUND_OFF);
        }
        SetKeySound(&song_state,LANE_2DRUM,p1_last_hitstate.drum_both ? KEYSOUND_2DRUM:KEYSOUND_OFF);
        SetKeySound(&song_state,LANE_2RIM,p1_last_hitstate.rim_both ? KEYSOUND_2RIM:KEYSOUND_OFF);
        
        // TODO: Observe Cursor State and, Based on Hit State, Judge any Notes, Update Judgement Structure with Judge Counts etc, Update life
        for(int i=0;i<8;i++){
            song_state.p1_judge_animation[i] = 0;
            song_state.p1_note_hit_animation[i] = 0;
        }
        
        for(int i=0;i<150;i++){
            if(((song_state.p1_cursor[i].cursor_flags & 2) > 0) && !song_state.p1_cursor[i].cursor_exflags){
                if(song_state.p1_cursor[i].cursor_ypos >= judge_min){
                    unsigned char lane = (song_state.p1_cursor[i].cursor_flags >> 6);
                    // If we've missed the judgement window, this is a miss.
                    if(song_state.p1_cursor[i].cursor_ypos > judge_max){
                        song_state.p1_judge_animation[lane] = ANI_JUDGE_MISS;
                        if(p1_judge.max_combo < song_state.p1_combo){
                         p1_judge.max_combo = song_state.p1_combo;   
                        }
                        song_state.p1_combo = 0;
                        p1_judge.miss++;
                        p1_judge.lifebar -= song_setting.level_rate_p1[JUDGE_MISS];
                        // Set the note to invisible now.
                        song_state.p1_cursor[i].cursor_flags &= ~2;                        
                        continue;
                    }
                    // Otherwise, we'll check if the lane is being hit. If it's not, we'll skip this for now.
                    unsigned char was_note_hit = 0;
                    switch(lane){
                        case LANE_BLUE:
                            was_note_hit = p1_last_hitstate.blue || p1_hitstate.blue;
                            break;
                        case LANE_DRUM:
                            was_note_hit = p1_last_hitstate.drum || p1_hitstate.drum;
                            break;
                        case LANE_2DRUM:
                            was_note_hit = p1_last_hitstate.drum_both || p1_hitstate.drum_both;
                            break;
                        case LANE_RIM:
                            was_note_hit = p1_last_hitstate.rim || p1_hitstate.rim;
                            break;
                        case LANE_2RIM:
                            was_note_hit = p1_last_hitstate.rim_both || p1_hitstate.rim_both;
                            break;
                        case LANE_RED:
                            was_note_hit = p1_last_hitstate.red || p1_hitstate.red;
                            break;
                        default:
                            break;
                    }
                    // If we're not hitting the note, we don't have to judge anything...
                    if(song_setting.p1_autoplay){
                        was_note_hit = 1;
                        switch(lane){
                            case LANE_BLUE:
                                IO_SET(swst,INP_P1_BLUE);
                                break;
                            case LANE_DRUM:
                                IO_SET(swst,INP_P1_DRUM_L);
                                break;
                            case LANE_2DRUM:
                                IO_SET(swst,INP_P1_DRUM_L);
                                IO_SET(swst,INP_P1_DRUM_R);
                                break;
                            case LANE_RIM:
                                IO_SET(swst,INP_P1_RIM_L);
                                break;
                            case LANE_2RIM:
                                IO_SET(swst,INP_P1_RIM_L);
                                IO_SET(swst,INP_P1_RIM_R);
                                break;
                            case LANE_RED:
                                IO_SET(swst,INP_P1_RED);
                                break;
                            default:
                                break;
                            
                        }
                      rmsg->button_io[0] = swst;  
                    }
                    if(!was_note_hit){continue;}
                    
                    song_state.p1_note_hit_animation[lane] = 1;
                    // If we have a fever flag, we add fever beat and the score.
                    if(song_state.p1_cursor[i].cursor_holdflags != 0){
                        unsigned char fever_combo_count = (song_state.p1_cursor[i].cursor_holdflags >> 2);
                        fever_combo_count++;                        
                        p1_judge.lifebar+= song_setting.level_rate_p1[JUDGE_FEVER];
                        song_state.p1_fever_beat++;
                        song_state.p1_cursor[i].cursor_holdflags <<= fever_combo_count;
                        song_state.p1_cursor[i].cursor_holdflags |= 2;
                        song_state.p1_score += (10 * song_state.p1_combo);
                        continue;
                    }
                    
                    // If we don't have a fever flag and it's a regular note
                    short ypos = song_state.p1_cursor[i].cursor_ypos;
                    // Hacky multiplier for double notes (2RIM 2DRUM count as 2 hits)
                    short hit_mult = 1;
                    if(lane == LANE_2DRUM || lane == LANE_2RIM){
                        hit_mult = 2; 
                    }                    
                    song_state.p1_combo+= hit_mult;

                    // Set the note to invisible now.
                    song_state.p1_cursor[i].cursor_flags &= ~2;                      
                    if((ypos >= judge_great_min && ypos <= judge_great_max) || song_setting.p1_autoplay){
                        song_state.p1_score += (5*song_state.p1_combo)  * hit_mult;
                        p1_judge.lifebar+= song_setting.level_rate_p1[JUDGE_GREAT]  * hit_mult;
                        song_state.p1_judge_animation[lane] = ANI_JUDGE_GREAT;
                        p1_judge.great+=hit_mult;
                    }else if(ypos >= judge_cool_min && ypos <= judge_cool_max){
                        song_state.p1_score += (4*song_state.p1_combo)  * hit_mult;
                        p1_judge.lifebar+= song_setting.level_rate_p1[JUDGE_COOL] * hit_mult;
                        song_state.p1_judge_animation[lane] = ANI_JUDGE_COOL;
                        p1_judge.cool+=hit_mult;
                        
                    }else if(ypos >= judge_nice_min && ypos <= judge_nice_max){
                        song_state.p1_score += (2*song_state.p1_combo)  * hit_mult;
                        p1_judge.lifebar+= song_setting.level_rate_p1[JUDGE_NICE]  * hit_mult;
                        song_state.p1_judge_animation[lane] = ANI_JUDGE_NICE;
                        p1_judge.nice+=hit_mult;
                        
                    }else{
                        song_state.p1_score += (1*song_state.p1_combo)  * hit_mult;
                        p1_judge.lifebar+= song_setting.level_rate_p1[JUDGE_POOR]  * hit_mult;
                        song_state.p1_judge_animation[lane] = ANI_JUDGE_POOR;
                        p1_judge.poor+=hit_mult;
                        
                    }
                
                    
                }                   
            }            
        }
        
        // Convert Float Lifebar to renderable one.
        if(p1_judge.lifebar > LIFEBAR_MAX){p1_judge.lifebar = LIFEBAR_MAX;}
        if(p1_judge.lifebar < LIFEBAR_MIN){p1_judge.lifebar = LIFEBAR_MIN;}
        song_state.p1_lifebar = (int)p1_judge.lifebar;
        p1_judge.hit_zero_life = (song_state.p1_lifebar == 0);

    }
    
    // TODO: Once we're sure the P1 stuff is perfect, copy it.
    if(song_state.p2_playing){
        song_state.p2_current_beat = current_beat;   
    } 
    
    // Copy Current State
    GetSongState(rmsg->data);
    // Reset Items for Next Read
    ResetSoundEvents(&song_state);
    p1_last_hitstate = p1_hitstate;
    p2_last_hitstate = p2_hitstate;
    last_swst = swst;

}


