#ifndef __SONG_H
#define __SONG_H

#include "a27.h"

enum SongMode{
	SONG_MODE_NORMAL,
	SONG_MODE_DEMO,
	SONG_MODE_OPENING,
	SONG_MODE_STAFF,
	SONG_MODE_HOWTOPLAY
};

typedef struct _SongSetting{
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
}SongSetting,*PSongSetting;

typedef struct _CURSOR_STATE{
    unsigned int cursor_flags;
    short cursor_ypos;
    short cursor_stretch;
}NOTECURSOR;

typedef struct _SONGSTATE{
    unsigned short cmd;
    unsigned short state;
    unsigned short p1_note_counter;
    unsigned short p2_note_counter;
    unsigned short sound_index[32];
    NOTECURSOR p1_cursor[150];
    NOTECURSOR p2_cursor[150];
    unsigned short p1_combo;
    unsigned short p2_combo;
    unsigned char p2_fireworks;
    unsigned char p2_fireworks_2;
    unsigned char p1_fireworks;
    unsigned char p1_fireworks_2;    
    unsigned char p1_playing;
    unsigned char p2_playing;
    unsigned char idk_maybepadding[2];
    unsigned char p1_judge_animation[8];
    unsigned char p2_judge_animation[8];
    unsigned char p1_lane_animation[8];
    unsigned char p2_lane_animation[8];
    unsigned char p1_note_hit_animation[8];
    unsigned char p2_note_hit_animation[8];
    unsigned int p1_score; 
    unsigned int p2_score; 
    unsigned int p1_score_2; 
    unsigned int p2_score_2; 
    unsigned int idk_maybepadding2; 
    unsigned short p1_lifebar;
    unsigned short p2_lifebar;
    unsigned short lifebar_align[2];
}SongState,*PSongState;

// This is basically a truncated start of the maingame setting struct.
typedef struct _SONGRESULT_REQ{
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
}SongResultRequest,*PSongResultRequest;

typedef struct _SONGRESULT{
    unsigned short cmd;
    unsigned char song_clear;
    unsigned char enable_bonus_stage;
    unsigned short p1_num_notes;
    unsigned short p2_num_notes;
    unsigned short p1_idk_1;
    unsigned short p2_idk_1;
    unsigned short p1_great;
    unsigned short p1_cool;
    unsigned short p1_nice;
    unsigned short p1_poor;
    unsigned short p1_lost;
    unsigned short p2_great;
    unsigned short p2_cool;
    unsigned short p2_nice;
    unsigned short p2_poor;
    unsigned short p2_lost;
    unsigned char  idk_block[0x14];
    unsigned short p1_max_combo;  
    unsigned short p2_max_combo;
    unsigned short p1_fever_beat;
    unsigned short p2_fever_beat;
    unsigned int   p1_idk_2;    
    unsigned int   p2_idk_2;
    unsigned int   p1_score;
    unsigned int   p2_score;
    unsigned char  p1_grade;
    unsigned char  p2_grade;
    unsigned char  p1_message;
    unsigned char  p2_message;
}SongResult,*PSongResult;

enum SongResultMessage{
    SongResulMessage_SOSO,   // This is just beating the song.
    SongResulMessage_BRAVO,  // Adds +15% to score <-- I think this is FC
    SongResulMessage_PERFECT // Adds +25% to score <-- I think this is PFC
};

enum SongResultGrade{
    ResultGradeS,
    ResultGradeAPlus,
    ResultGradeA,
    ResultGradeAMinus,
    ResultGradeBPlus,
    ResultGradeB,
    ResultGradeBMinus,
    ResultGradeCPlus,
    ResultGradeC,
    ResultGradeCMinus,
    ResultGradeDPlus,
    ResultGradeD,
    ResultGradeDMinus,
    ResultGradeE
};

void Song_UploadPlaybackHeader(const unsigned char* in_data, struct A27_Read_Message* msg);
void Song_UploadPlaybackBody(const unsigned char* in_data, struct A27_Read_Message* msg);
void Song_MainGameSetting(const unsigned char* in_data, struct A27_Read_Message* msg);
void Song_MainGameWaitStart(const unsigned char* in_data, struct A27_Read_Message* msg);
void Song_MainGameStart(const unsigned char* in_data, struct A27_Read_Message* msg);
void Song_MainGameProcess(const unsigned char* in_data, struct A27_Read_Message* msg);
void Song_ResultProcess(const unsigned char* in_data, struct A27_Read_Message* msg);

#endif
