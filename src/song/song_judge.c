#include <stdio.h>
#include <string.h>


#include "song_settings.h"

#include "song_judge.h"




unsigned char score_multiplier[5] = {10,5,4,2,1};

void SongJudgeInit(PSongJudge judge, PSongSettings song_settings){
    memset(judge,0x00,sizeof(SongJudge));
    
    judge->player[0].lifebar = LIFEBAR_START;
    judge->player[1].lifebar = LIFEBAR_START;
    
    // Set up Judgement Offsets
    judge->settings.offsets.miss.min = JUDGE_MIN;
    judge->settings.offsets.miss.max = JUDGE_MAX;
    judge->settings.offsets.great.max = JUDGE_CENTER + song_settings->judge.great;
    judge->settings.offsets.great.min = JUDGE_CENTER - song_settings->judge.great;
    judge->settings.offsets.cool.max = JUDGE_CENTER + song_settings->judge.cool;
    judge->settings.offsets.cool.min = JUDGE_CENTER - song_settings->judge.cool;
    judge->settings.offsets.nice.max = JUDGE_CENTER + song_settings->judge.nice;
    judge->settings.offsets.nice.min = JUDGE_CENTER - song_settings->judge.nice;
    judge->settings.offsets.poor.max = JUDGE_CENTER + song_settings->judge.poor;
    judge->settings.offsets.poor.min = JUDGE_CENTER - song_settings->judge.poor;    

    printf("Song Judge Ranges:  Great: %d->%d Cool: %d->%d Nice: %d->%d Poor: %d->%d Miss: %d->%d\n",judge->settings.offsets.great.min,judge->settings.offsets.great.max,judge->settings.offsets.cool.min,judge->settings.offsets.cool.max,judge->settings.offsets.nice.min,judge->settings.offsets.nice.max,judge->settings.offsets.poor.min,judge->settings.offsets.poor.max,judge->settings.offsets.miss.min,judge->settings.offsets.miss.max);
    
    for(int i=0;i<2;i++){
        judge->settings.lifebar_rate[i].fever = song_settings->level_rate[i].fever;
        judge->settings.lifebar_rate[i].great = song_settings->level_rate[i].great;
        judge->settings.lifebar_rate[i].cool = song_settings->level_rate[i].cool;
        judge->settings.lifebar_rate[i].nice = song_settings->level_rate[i].nice;
        judge->settings.lifebar_rate[i].poor = song_settings->level_rate[i].poor;
        judge->settings.lifebar_rate[i].miss = song_settings->level_rate[i].miss;
    }
    
}

unsigned char FeverJudge(PSongJudge judge, short cursor_y, short cursor_offset, unsigned char fever_amount, unsigned char player_hit_state, unsigned char player_index, unsigned char player_autoplay, unsigned short total_notes){
    unsigned char judge_ani = ANI_JUDGE_NONE;
    // If we're not hitting it (or on autoplay) we don't care
    if(!player_hit_state && !player_autoplay){return judge_ani;}
    
    short cursor_min = cursor_y + cursor_offset;
    short cursor_max = cursor_y;
    
    // If we're not at the beat zone yet, we also don't care
    if(cursor_max < judge->settings.offsets.poor.min){return judge_ani;}
    
    // If we're past the beat zone, we also don't care (we'll cull this later). This also resets your fever combo because we're done the fever.
    // We'll set the animation to a fake "FEVER_OVER" enum so we'll cull the note at this point too.
    if(cursor_min > judge->settings.offsets.poor.max){
        judge->player[player_index].current_fever_combo = 0;
        judge_ani = ANI_JUDGE_FEVER_OVER;
        return judge_ani;
        
    }
    
    
    float lifebar_rate = judge->settings.lifebar_rate[player_index].fever;
    
    // Update Lifebar
    judge->player[player_index].lifebar += lifebar_rate ;
    // Snap Lifebar to Max if Necessary.
    if(judge->player[player_index].lifebar > LIFEBAR_MAX){
        judge->player[player_index].lifebar = LIFEBAR_MAX;
    }
    
    judge_ani = ANI_JUDGE_FEVER_HIT;
        
    // If we full combo'd the song, we'll send a different animation back.
    if(total_notes == judge->player[player_index].hit_combo){
        judge_ani = ANI_JUDGE_BRAVO;
    }  

    judge->player[player_index].current_fever_combo++;
    
    // If we haven't exceeded the stated amount of fever combo - increment.
    if(judge->player[player_index].current_fever_combo <= fever_amount){
        judge->player[player_index].hit_combo++;
        judge->player[player_index].fever++;
    }
    
    // Update Score
    judge->player[player_index].score += (score_multiplier[JUDGE_FEVER] * judge->player[player_index].hit_combo);
    
    
     
    return judge_ani;
    
}

// This is executed on every non-hitless, non-fever cursor to determine if the player hit the cursor and update their state accordingly.
// It returns an ANI_JUDGE enum to tell the animation if a judgement note hit needs to be shown.
unsigned char CursorJudge(PSongJudge judge, short cursor_y, unsigned char track_index, unsigned char player_hit_state, unsigned char player_index, unsigned char player_autoplay, unsigned short total_notes){

    // If it's outside of the judgement zone, we don't care.
    if(cursor_y < judge->settings.offsets.poor.min){return ANI_JUDGE_NONE;}

    // To support autoplay, we'll also return until we're in the "GREAT" range.
    if(player_autoplay && cursor_y < judge->settings.offsets.great.min){        
        return ANI_JUDGE_NONE;
    }
    
    // To account for double-hit cursors, we have to increment the beat count.
    int hit_inc = 1;
    if(track_index == 2 || track_index == 4){
            hit_inc++;
    }
    
    if(cursor_y > judge->settings.offsets.poor.max){
        // Record Max Combo if necessary and reset Hit Combo
        if(judge->player[player_index].max_combo < judge->player[player_index].hit_combo){
            judge->player[player_index].hit_combo = judge->player[player_index].max_combo;   
        }
        judge->player[player_index].hit_combo = 0;
        // Increment Our Miss Count
        judge->player[player_index].miss+=hit_inc;
        
        // Update Our Current Lifebar 
        judge->player[player_index].lifebar += (judge->settings.lifebar_rate[player_index].miss * hit_inc);
        
        // Snap to 0 if Lifebar is < 0 and if we hit zero, mark it for results later.
        if(judge->player[player_index].lifebar < LIFEBAR_MIN){
            judge->player[player_index].lifebar = LIFEBAR_MIN;
        }
        if(judge->player[player_index].lifebar == LIFEBAR_MIN){
            judge->player[player_index].lifebar_hit_zero = 1;
        }
        return ANI_JUDGE_MISS;
    }
    
    // If we haven't missed a cursor, yet we're not hitting one at the moment, we'll skip everything else.
    if(!player_hit_state && !player_autoplay){return ANI_JUDGE_NONE;}

    unsigned char judge_ani = ANI_JUDGE_NONE;
    float lifebar_rate = 0;
    unsigned int score_mult = 0;
    
    // Now, we'll cascade the different judgements.
    if(cursor_y >= judge->settings.offsets.great.min && cursor_y <= judge->settings.offsets.great.max){
        judge->player[player_index].great+=hit_inc;
        lifebar_rate = judge->settings.lifebar_rate[player_index].great;
        score_mult = score_multiplier[JUDGE_GREAT];
        judge_ani = ANI_JUDGE_GREAT;
    }else if(cursor_y >= judge->settings.offsets.cool.min && cursor_y <= judge->settings.offsets.cool.max){
        judge->player[player_index].cool+=hit_inc;
        lifebar_rate = judge->settings.lifebar_rate[player_index].cool;
        score_mult = score_multiplier[JUDGE_COOL];
        judge_ani = ANI_JUDGE_COOL;
    }else if(cursor_y >= judge->settings.offsets.nice.min && cursor_y <= judge->settings.offsets.nice.max){
        judge->player[player_index].nice+=hit_inc;
        lifebar_rate = judge->settings.lifebar_rate[player_index].nice;
        score_mult = score_multiplier[JUDGE_NICE];
        judge_ani = ANI_JUDGE_NICE;
    }else if(cursor_y >= judge->settings.offsets.poor.min && cursor_y <= judge->settings.offsets.poor.max){
        judge->player[player_index].poor+=hit_inc;
        lifebar_rate = judge->settings.lifebar_rate[player_index].poor;
        score_mult = score_multiplier[JUDGE_POOR];
        judge_ani = ANI_JUDGE_POOR;
    }    
    
    // Update Score
    judge->player[player_index].score += (score_mult * judge->player[player_index].hit_combo);
    
    // Update Hit Combo
    judge->player[player_index].hit_combo+=hit_inc;
    
    // Update Lifebar
    judge->player[player_index].lifebar += (lifebar_rate * hit_inc);
    // Snap Lifebar to Max if Necessary.
    if(judge->player[player_index].lifebar > LIFEBAR_MAX){
        judge->player[player_index].lifebar = LIFEBAR_MAX;
    }
        
    // If we full combo'd the song, we'll send a different animation back.
    if(total_notes == judge->player[player_index].hit_combo){
        judge_ani = ANI_JUDGE_BRAVO;
    }        
    //printf("Return Judge :%d\n",judge_ani);
    return judge_ani;
}
