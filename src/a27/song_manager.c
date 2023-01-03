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

// Debug Helpers
void PrintSongSetting(void){
    printf("--- New Song Setting ---\n");
    printf("State: %d\n",song_setting.state);
    printf("Stage: %d GameMode: %d KeyRecord:%d\n",song_setting.stage_num,song_setting.game_mode,song_setting.key_record_mode);
    printf("P1 Enable: %d Version: %d SongID: %d\n",song_setting.p1_enable,song_setting.p1_songversion,song_setting.p1_songid);
    printf("P1 Speed: %d Cloak: %d Noteskin: %d Auto: %d\n",song_setting.p1_speed,song_setting.p1_cloak,song_setting.p1_noteskin,song_setting.p1_autoplay);
    printf("P2 Enable: %d Version: %d SongID: %d\n",song_setting.p2_enable,song_setting.p2_songversion,song_setting.p2_songid);
    printf("P2 Speed: %d Cloak: %d Noteskin: %d Auto: %d\n",song_setting.p2_speed,song_setting.p2_cloak,song_setting.p2_noteskin,song_setting.p2_autoplay);
    printf("Scoring: G: %d C: %d N: %d P: %d\n",song_setting.judge_great,song_setting.judge_cool,song_setting.judge_nice,song_setting.judge_poor);
    printf("P1 Rating: %d P2 Rating: %d\n",song_setting.p1_rating,song_setting.p2_rating);
    printf("LevelRate P1: Fever %.2f Great %.2f Cool %.2f Nice %.2f Poor %.2f Miss %.2f\n",song_setting.level_rate_p1[0],song_setting.level_rate_p1[1],song_setting.level_rate_p1[2],song_setting.level_rate_p1[3],song_setting.level_rate_p1[4],song_setting.level_rate_p1[5]);
    printf("LevelRate P2: Fever %.2f Great %.2f Cool %.2f Nice %.2f Poor %.2f Miss %.2f\n",song_setting.level_rate_p2[0],song_setting.level_rate_p2[1],song_setting.level_rate_p2[2],song_setting.level_rate_p2[3],song_setting.level_rate_p2[4],song_setting.level_rate_p2[5]);
    printf("IDK 1 [Probably Padding]: ");
    for(int i=0;i<8;i++){
     printf("%02X",song_setting.idk_1[i]);   
    }
    printf("\n");
    printf("IDK P1: %d P2: %d\n",song_setting.idk_p1_1,song_setting.idk_p1_2);
    printf("Is Non Challenge Mode: %d\n",song_setting.is_non_challengemode);
    printf("Song Mode: %d\n",song_setting.song_mode);
    printf("--- End Song Setting ---\n");
}

void PrintSongInfo(void){
    printf(" --- Song Event File (SEF) Loaded! --- \n");
    printf(" Tempo: %.02f Beats: %d Notes: %d/%d SoundEvents: %d CursorEvents: %d\n",song_info.header.tempo,song_info.header.total_beats,song_info.header.min_notes,song_info.header.total_notes,song_info.header.total_soundevents,song_info.header.total_cursorevents);
    printf(" --- End Song Event File --- \n");
}

void GetSongInfo(unsigned int song_mode, unsigned int song_id){
    char sef_path[1024] = {0x00};
    const char* sef_template = "event/%d_%d.sef";
    sprintf(sef_path,sef_template,song_mode,song_id);
    FILE* fp = fopen(sef_path,"rb");
    if(!fp){
     printf("Error! Could not Open SEF File: %s\n",sef_path);
     exit(-1);
    }
    
    // Load Header
    fread(&song_info,sizeof(SongInfoHeader),1,fp);
    
    if(song_info.header.total_soundevents){
        song_info.sound_events = (PSoundEvent)malloc(song_info.header.total_soundevents * sizeof(SoundEvent));
    }
    
    for(int i = 0; i < song_info.header.total_soundevents;i++){
        fread(song_info.sound_events+i,sizeof(SoundEvent),1,fp);
    }

    if(song_info.header.total_cursorevents){
        song_info.cursor_events = (PCursorEvent)malloc(song_info.header.total_cursorevents * sizeof(CursorEvent));
    }
    for(int i = 0; i < song_info.header.total_cursorevents; i++){
        fread(song_info.cursor_events+i,sizeof(CursorEvent),1,fp);        
    }
    
    fclose(fp);    
    PrintSongInfo();
}



// --- Interface ---
void GetSongState(void* state){    
    memcpy(state,&song_state,sizeof(SongState));
}

// We'll come up with a way to calculate the grade based on FPC and
// give a value to each judgement as 4,3,2,1;
unsigned char CalculateGrade(PPlayerJudge judge){
    // First, calculate the max value.
    unsigned int max_nval = song_info.header.total_notes * 4; // where 4 is all "GREAT"
    unsigned int jval = (judge->great * 4) + (judge->cool * 3) + (judge->nice * 2) + (judge->poor * 1);
    float grade = ((float)jval / (float)max_nval) * 100.f;
    if(grade > 99.9f){return ResultGradeS;}
    if(grade > 96.9f){return ResultGradeAPlus;}
    if(grade > 93.0f){return ResultGradeA;}
    if(grade > 89.9f){return ResultGradeAMinus;}
    if(grade > 86.9f){return ResultGradeBPlus;}
    if(grade > 82.9f){return ResultGradeB;}
    if(grade > 79.9f){return ResultGradeBMinus;}
    if(grade > 76.9f){return ResultGradeCPlus;}
    if(grade > 72.9f){return ResultGradeC;}
    if(grade > 69.9f){return ResultGradeCMinus;}    
    if(grade > 66.9f){return ResultGradeDPlus;}
    if(grade > 62.9f){return ResultGradeD;}
    if(grade > 59.9f){return ResultGradeDMinus;}
    
    return ResultGradeE;
}

void GetSongResult(void* request, void* response){
    SongResult res = {0};
    PSongResultRequest req = request;
    res.cmd = A27_SONGMODE_RESULT;
    unsigned char p1_clear = 0;
    unsigned char p2_clear = 0;
    
    
    if(song_setting.p1_enable){
        unsigned int total_p1_player_notes = p1_judge.great + p1_judge.cool + p1_judge.nice + p1_judge.poor;
        res.p1_num_notes = song_info.header.total_notes;    
        res.p1_fever_beat = song_state.p1_fever_beat;
        res.p1_great = p1_judge.great;
        res.p1_cool = p1_judge.cool;
        res.p1_nice = p1_judge.nice;
        res.p1_poor = p1_judge.poor;
        res.p1_miss = p1_judge.miss;
        res.p1_score = song_state.p1_score;
        res.p1_max_combo = p1_judge.max_combo;
        if(res.p1_max_combo < song_state.p1_combo){
            res.p1_max_combo = song_state.p1_combo;
        }
        res.p1_grade = CalculateGrade(&p1_judge);
        p1_clear = (total_p1_player_notes >= song_info.header.min_notes) ? 1:0;
        // Calculating Message
        if(p1_clear){
            res.p1_message = SongResulMessage_SOSO;
        }
        if(res.p1_num_notes == res.p1_max_combo){
            res.p1_message = SongResulMessage_BRAVO;
            if(res.p1_max_combo == res.p1_great){
                res.p1_message = SongResulMessage_PERFECT;
            }
        }
    }
    
    if(song_setting.p2_enable){
        unsigned int total_p2_player_notes = p2_judge.great + p2_judge.cool + p2_judge.nice + p2_judge.poor;
        res.p2_num_notes = song_info.header.total_notes;
        res.p2_fever_beat = song_state.p2_fever_beat;
        res.p2_great = p2_judge.great;
        res.p2_cool = p2_judge.cool;
        res.p2_nice = p2_judge.nice;
        res.p2_poor = p2_judge.poor;
        res.p2_miss = p2_judge.miss;
        res.p2_score = song_state.p2_score;
        res.p2_max_combo = p2_judge.max_combo;
        res.p2_grade = CalculateGrade(&p2_judge);
        p2_clear = (total_p2_player_notes >= song_info.header.min_notes) ? 1:0;
        // Calculating Message
        if(p2_clear){
            res.p2_message = SongResulMessage_SOSO;
        }
        if(res.p2_num_notes == res.p2_max_combo){
            res.p2_message = SongResulMessage_BRAVO;
            if(res.p2_max_combo == res.p2_great){
                res.p2_message = SongResulMessage_PERFECT;
            }
        }
    }
    
    
    res.song_clear = (p1_clear || p2_clear) ? 1:0;
    res.enable_bonus_stage = (res.song_clear && song_setting.stage_num == 2) ? 1 : 0;
  
    memcpy(response,&res,sizeof(SongResult));
    
}

void SongManager_Init(void* setting){
    printf("SongManager_Init\n");
    memcpy(&song_setting,setting,sizeof(SongSetting));
    judge_min = JUDGE_CENTER - song_setting.judge_poor;
    judge_max = JUDGE_CENTER + song_setting.judge_poor;
    judge_great_min = JUDGE_CENTER - song_setting.judge_great;
    judge_great_max = JUDGE_CENTER + song_setting.judge_great;
    judge_cool_min = JUDGE_CENTER - song_setting.judge_cool;
    judge_cool_max = JUDGE_CENTER + song_setting.judge_cool;
    judge_nice_min =  JUDGE_CENTER - song_setting.judge_nice;
    judge_nice_max =  JUDGE_CENTER - song_setting.judge_nice;
    judge_poor_min = judge_min;
    judge_poor_max =  judge_max;
    memset(&song_state,0,sizeof(SongState));
    memset(&p1_judge,0,sizeof(PlayerJudge));
    memset(&p2_judge,0,sizeof(PlayerJudge));    
    song_state.cmd = A27_SONGMODE_MAINGAME_SETTING;
}

void SongManager_Reset(void){
    printf("SongManager_Reset\n");
    song_state.cmd = A27_SONGMODE_MAINGAME_WAITSTART;
    if(song_setting.p1_enable){
        song_state.p1_playing = 1;
        song_state.p1_current_beat = -1;    
        song_state.p1_lifebar = LIFEBAR_START;
        p1_judge.lifebar = (float)song_state.p1_lifebar;
    }
    if(song_setting.p2_enable){
        song_state.p2_playing = 1;
        song_state.p2_current_beat = -1;
        song_state.p2_lifebar = LIFEBAR_START;
        p2_judge.lifebar = (float)song_state.p2_lifebar;
        
    }
}

void SongManager_Start(void){
    printf("SongManager_Start\n");
    // Get Song Info from Event File
    unsigned int current_song_id = (song_setting.p1_songid > song_setting.p2_songid) ? song_setting.p1_songid : song_setting.p2_songid;
    GetSongInfo(song_setting.song_mode,current_song_id);
    
    PrintSongSetting();
    song_state.cmd = A27_SONGMODE_MAINGAME_PROCESS;
    // TODO: Start the Timers, Scrolling, All that shit.
    sparms.song_info = &song_info;
    sparms.song_setting = &song_setting;
    sparms.song_state = &song_state;
    sparms.p1_judge = &p1_judge;
    sparms.p2_judge = &p2_judge;
    StartTimer(&sparms);
    
}

void SongManager_Stop(void){
    printf("SongManager_Stop\n");
 // TODO: Stop the Timers, Scrolling, All that shit.   
    StopTimer();
}


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


