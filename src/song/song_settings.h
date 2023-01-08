#ifndef __SONG_SETTINGS_H
#define __SONG_SETTINGS_H

typedef struct _PLAYER_MOD{
    unsigned char speed;
    unsigned char cloak;
    unsigned char noteskin;
    unsigned char align;
}PlayerMod,*PPlayerMod;

typedef struct _JUDGE_MARGIN{
 unsigned short great;
 unsigned short cool;
 unsigned short nice;
 unsigned short poor;
 unsigned short align;
}JudgeMargin,*PJudgeMargin;

typedef struct _JUDGE_LEVEL_RATE{
    float fever;
    float great;
    float cool;
    float nice;
    float poor;
    float miss;
}JudgeLevelRate,*PJudgeLevelRate;


typedef struct _SONG_SETTINGS{
    unsigned short cmd;
    unsigned char state;
    unsigned char stage_num;
    unsigned char game_mode;
    unsigned char key_record_mode;
    unsigned char player_enable[2];
    unsigned char player_autoplay[2];
    unsigned char player_song_version[2];
    unsigned short player_chartid[2];
    PlayerMod player_mod[2];    
    JudgeMargin judge;
    unsigned char player_rating_level[2];
    JudgeLevelRate level_rate[2];
    unsigned char idk[10];    
    unsigned char is_non_challengemode;
    unsigned char song_mode;
}SongSettings,*PSongSettings;



void PrintSongSetting(PSongSettings song_setting);
#endif
