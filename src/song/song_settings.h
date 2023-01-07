#ifndef __SONG_SETTINGS_H
#define __SONG_SETTINGS_H



typedef struct _SONG_SETTINGS{
    unsigned short cmd;
    unsigned char state;
    unsigned char stage_num;
    unsigned char game_mode;
    unsigned char key_record_mode;
    unsigned char p1_enable;
    unsigned char p2_enable;
    unsigned char p1_autoplay;
    unsigned char p2_autoplay;
    unsigned char p1_songversion;
    unsigned char p2_songversion;
    unsigned short p1_songid;
    unsigned short p2_songid;
    unsigned char p1_speed;
    unsigned char p1_cloak;
    unsigned char p1_noteskin;
    unsigned char align_1;
    unsigned char p2_speed;
    unsigned char p2_cloak;
    unsigned char p2_noteskin;
    unsigned char align_2;
    unsigned short judge_great; 
    unsigned short judge_cool;
    unsigned short judge_nice;
    unsigned short judge_poor;
    unsigned short align_3;
    unsigned char p1_rating;
    unsigned char p2_rating;
    float level_rate_p1[6];
    float level_rate_p2[6];
    unsigned char idk_1[8];
    unsigned char idk_p1_1;
    unsigned char idk_p1_2;
    unsigned char is_non_challengemode;
    unsigned char song_mode;
}SongSettings,*PSongSettings;




#endif
