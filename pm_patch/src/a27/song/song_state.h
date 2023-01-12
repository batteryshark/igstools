#ifndef __SONG_STATE_H
#define __SONG_STATE_H

#include "song_cursor.h"

enum CURSOR_LANE{
    LANE_BLUE,
    LANE_DRUM,
    LANE_2DRUM,
    LANE_RIM,
    LANE_2RIM,
    LANE_RED,
    LANE_PLACEHOLDER_1,
    LANE_PLACEHOLDER_2
};

enum KeySoundIndex{
    KEYSOUND_OFF,
    KEYSOUND_BLUE,
    KEYSOUND_RED,
    KEYSOUND_DRUM,
    KEYSOUND_RIM,
    KEYSOUND_2DRUM,
    KEYSOUND_2RIM
};

typedef struct _PLAYER_ANIMATION{
    unsigned char track[8];
}PlayerAnimation,*PPLayerAnimation;

typedef struct _PLAYER_HIT_STATE{
    unsigned char track[8];
}PlayerHitState,*PPlayerHitState;


typedef struct _SONGSTATE{
    unsigned short cmd;
    unsigned short state;
    unsigned short current_beat[2];
    unsigned short sound_index[32];
    PlayerCursor player_cursor[2];
    unsigned short player_combo[2];
    unsigned short player_fever_combo[2];
    unsigned char player_isplaying[2];
    unsigned char player_fever_beat[2];
    PlayerAnimation player_judge_graphic[2];
    PlayerAnimation player_track_hit_animation[2];
    PlayerAnimation player_cursor_hit_animation[2];
    unsigned int player_score[2];
    unsigned int player_score_copy[2];    
    unsigned int idk_maybepadding2; 
    unsigned short player_life[2];    
    unsigned short lifebar_align[2];
}SongState,*PSongState;


#endif
