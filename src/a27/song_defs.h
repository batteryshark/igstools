#ifndef __SONG_DEFS_H
#define __SONG_DEFS_H




#define JUDGE_CENTER 0x179
#define CURSOR_MAX_Y 0x1A0
enum KeySoundIndex{
    KEYSOUND_OFF,
    KEYSOUND_BLUE,
    KEYSOUND_RED,
    KEYSOUND_DRUM,
    KEYSOUND_RIM,
    KEYSOUND_2DRUM,
    KEYSOUND_2RIM
};



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
    ANI_JUDGE_PLACEHOLDER_2
};



enum SongMode{
	SONG_MODE_NORMAL,
	SONG_MODE_DEMO,
	SONG_MODE_OPENING,
	SONG_MODE_STAFF,
	SONG_MODE_HOWTOPLAY
};

enum LifebarSetting{
    LIFEBAR_MIN,
    LIFEBAR_START=10,
    LIFEBAR_PASS=19,
    LIFEBAR_MAX=28
};




typedef struct _SongInfoHeader{
    float tempo;
    unsigned int total_beats;
    unsigned int total_notes;
    unsigned int min_notes;
    unsigned int total_soundevents;
    unsigned int total_cursorevents;
}SongInfoHeader,*PSongInfoHeader;

typedef struct _SongInfo{
    SongInfoHeader header;
    void* sound_events;
    void* cursor_events;
}SongInfo,*PSongInfo;

// -- End Unofficial Stuff

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
    unsigned short flags;
    unsigned char exflags;
    unsigned char fever_flags;
    short ypos;
    short fever_offset;
}NoteCursor,*PNoteCursor;

typedef struct _SONGSTATE{
    unsigned short cmd;
    unsigned short state;
    unsigned short p1_current_beat;
    unsigned short p2_current_beat;
    unsigned short sound_index[32];
    NoteCursor p1_cursor[150];
    NoteCursor p2_cursor[150];
    unsigned short p1_combo;
    unsigned short p2_combo;
    unsigned short p1_fever_beat;
    unsigned short p2_fever_beat;    
    unsigned char p1_playing;
    unsigned char p2_playing;
    unsigned char p1_holdcombo;
    unsigned char p2_holdcombo;
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

typedef struct _PLAYER_JUDGECOUNT{
    float lifebar;
    unsigned char hit_zero_life;
    unsigned int max_combo;
    unsigned int great;
    unsigned int cool;
    unsigned int nice;
    unsigned int poor;
    unsigned int miss;
}PlayerJudge,*PPlayerJudge;

typedef struct _PLAYER_HITSTATE{
    unsigned char blue;
    unsigned char drum;
    unsigned char drum_l;
    unsigned char drum_r;
    unsigned char drum_both;
    unsigned char rim;
    unsigned char rim_l;
    unsigned char rim_r;
    unsigned char rim_both;
    unsigned char red;
}PlayerHitState,*PPlayerHitState;

typedef struct _SONG_PARAMS{
 PSongState song_state;
 PSongSetting song_setting;
 PSongInfo song_info;
 PPlayerJudge p1_judge;
 PPlayerJudge p2_judge;
}SongParams,*PSongParams;

// --- Result Defines
enum SongResultMessage{
    SongResulMessage_SOSO,   // This is just beating the song.
    SongResulMessage_BRAVO,  // Adds +15% to score <-- I think this is FC
    SongResulMessage_PERFECT // Adds +25% to score <-- I think this is PFC
};



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
    unsigned short p1_miss;
    unsigned short p2_great;
    unsigned short p2_cool;
    unsigned short p2_nice;
    unsigned short p2_poor;
    unsigned short p2_miss;
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

#endif
