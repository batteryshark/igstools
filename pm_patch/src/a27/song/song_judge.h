#ifndef __SONG_JUDGE_H
#define __SONG_JUDGE_H

#include "song_settings.h"

#define JUDGE_CENTER 0x171
#define JUDGE_MAX JUDGE_CENTER + 50
#define JUDGE_MIN JUDGE_CENTER - 50

#define CURSOR_MAX_YVAL 512

enum JUDGE_RATE{
    JUDGE_FEVER,
    JUDGE_GREAT,
    JUDGE_COOL,
    JUDGE_NICE,
    JUDGE_POOR,
    JUDGE_MISS
};

enum ANI_JUDGE{
    ANI_JUDGE_NONE,
    ANI_JUDGE_GREAT,
    ANI_JUDGE_COOL,
    ANI_JUDGE_NICE,
    ANI_JUDGE_POOR,
    ANI_JUDGE_MISS,
    ANI_JUDGE_BRAVO,
    ANI_JUDGE_PLACEHOLDER_1,
    ANI_JUDGE_PLACEHOLDER_2,
    ANI_JUDGE_FEVER_HIT,// THIS IS NOT A REAL ENUM
    ANI_JUDGE_FEVER_OVER // THIS IS NOT A REAL ENUM
};


    
typedef struct _JUDGE_LIFEBAR_RATE{
    float fever;
    float great;
    float cool;
    float nice;
    float poor;
    float miss;
}JudgeLifeBarRate,*PJudgeLifeBarRate;

enum LifebarSetting{
    LIFEBAR_MIN,
    LIFEBAR_START=10,
    LIFEBAR_PASS=19,
    LIFEBAR_MAX=28
};



typedef struct _JUDGE_RANGE{
    unsigned short min;
    unsigned short max;
}JudgeRange,*PJudgeRange;

typedef struct _JUDGE_OFFSETS{
    JudgeRange great;
    JudgeRange cool;
    JudgeRange nice;
    JudgeRange poor;
    JudgeRange miss;    
}JudgeOffsets,*PJudgeOffsets;

typedef struct _JUDGE_SETTINGS{
    JudgeOffsets offsets;  
    JudgeLifeBarRate lifebar_rate[2];
}JudgeSettings,*PJudgeSettings;

typedef struct _PLAYER_JUDGECOUNT{
    float lifebar;
    unsigned char lifebar_hit_zero;
    unsigned int hit_combo;
    unsigned int current_fever_combo;
    unsigned int max_combo;
    unsigned int fever;
    unsigned int great;
    unsigned int cool;
    unsigned int nice;
    unsigned int poor;
    unsigned int miss;
    unsigned int score;
}PlayerJudge,*PPlayerJudge;

typedef struct _SONG_JUDGE{
    JudgeSettings settings;   
    PlayerJudge player[2];
}SongJudge,*PSongJudge;



void SongJudgeInit(PSongJudge judge,PSongSettings song_settings);
unsigned char FeverJudge(PSongJudge judge,short cursor_y, short cursor_offset, unsigned char fever_amount, unsigned char player_hit_state, unsigned char player_index, unsigned char player_autoplay, unsigned short total_notes);
unsigned char CursorJudge(PSongJudge judge,short cursor_y, unsigned char track_index, unsigned char player_hit_state, unsigned char player_index, unsigned char player_autoplay, unsigned short total_notes);
#endif
