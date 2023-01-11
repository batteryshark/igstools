#ifndef __SONG_RESULT_H
#define __SONG_RESULT_H

#include "song_settings.h"
#include "song_judge.h"

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

// This is basically a truncated start of the maingame setting struct.
// This isn't necessary as we already have this state info from earlier.
typedef struct _SONGRESULT_REQ{
    unsigned short cmd;
    unsigned char state;
    unsigned char stage_num;
    unsigned char game_mode;
    unsigned char key_record_mode;
    unsigned char player_enable[2];
    unsigned char player_autoplay[2];
    unsigned char player_songversion[2];
    unsigned short player_chartid[2];
    unsigned char p1_speed;
    unsigned char p1_cloak;
    unsigned char p1_noteskin;
    unsigned char align_1;
}SongResultRequest,*PSongResultRequest;

typedef struct _RESULT_PLAYER_HIT{
    unsigned short great;
    unsigned short cool;
    unsigned short nice;
    unsigned short poor;
    unsigned short miss;
}ResultPlayerHit,*PResultPlayerHit;


typedef struct _SONGRESULT{
    unsigned short cmd;
    unsigned char song_clear;
    unsigned char enable_bonus_stage;
    unsigned short player_num_notes[2];
    unsigned short player_idk[2];    
    ResultPlayerHit player_hit[2];
    unsigned char  idk_block[0x14];
    unsigned short player_max_combo[2];  
    unsigned short player_fever_beat[2];
    unsigned int   player_idk_2[2];    
    unsigned int player_score[2];
    unsigned char player_grade[2];
    unsigned char player_message[2];
}SongResult,*PSongResult;


void GetSongResult(PSongSettings settings,PSongJudge judge,PSongResult res, unsigned short total_notes);

#endif
