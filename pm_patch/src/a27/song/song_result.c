#include <stdio.h>
#include <string.h>

#include "song_judge.h"
#include "song_settings.h"

#include "song_result.h"


// This calculates a letter grade based off of the percentage that you were to FPC
unsigned char CalculateGrade(PSongJudge judge, unsigned char player_index, unsigned short total_notes){
    // First, calculate the max value.    
    unsigned int max_nval = total_notes * 4; // where 4 is all "GREAT" or "FEVER"
    unsigned int jval = (judge->player[player_index].fever * 4) + (judge->player[player_index].great * 4) + (judge->player[player_index].cool * 3) + (judge->player[player_index].nice * 2) + (judge->player[player_index].poor * 1);
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


void GetSongResult(PSongSettings settings, PSongJudge judge,PSongResult res, unsigned short total_notes){
    
    for(int i=0;i<2;i++){        
        if(!settings->player_enable[i]){continue;}        
        unsigned int player_clear = 0;
        res->player_num_notes[i] = total_notes;
        res->player_fever_beat[i] = judge->player[i].total_fever_hits;
        res->player_hit[i].great = judge->player[i].great;
        res->player_hit[i].cool = judge->player[i].cool;
        res->player_hit[i].nice = judge->player[i].nice;
        res->player_hit[i].poor = judge->player[i].poor;
        res->player_hit[i].miss = judge->player[i].miss;
        res->player_score[i] = judge->player[i].score;
        // Update Max Combo if we never hit a break
        res->player_max_combo[i] = (judge->player[i].max_combo > judge->player[i].hit_combo) ? judge->player[i].max_combo : judge->player[i].hit_combo;
        res->player_grade[i] = CalculateGrade(judge,i,total_notes);
        player_clear = ((res->player_grade[i] > ResultGradeDMinus) || judge->player[i].lifebar_hit_zero) ? 0 : 1;
        // Calculate Message - If you Pass you Get SoSo, If you FC you Get Bravo, FPC PERFECT
        if(player_clear){
            res->player_message[i] = SongResulMessage_SOSO;
        }
        if(res->player_num_notes[i] == res->player_max_combo[i]){
            res->player_message[i] = SongResulMessage_BRAVO;
        }
        if(res->player_num_notes[i] == (res->player_hit[i].great + judge->player[i].fever)){
            res->player_message[i] = SongResulMessage_PERFECT;
        }
        // If either player clears, we'll just mark clear.
        if(player_clear){res->song_clear = 1;}
        
    }
    // TODO: Verify this actually works...
    res->enable_bonus_stage = (res->song_clear && settings->stage_num > 2) ? 1 : 0;
}
