#ifndef __SONG_MANAGER_H
#define __SONG_MANAGER_H



enum A27_Song_Subcommand{
 A27_SONGMODE_PLAYBACK_HEADER,
 A27_SONGMODE_PLAYBACK_BODY,
 A27_SONGMODE_2,
 A27_SONGMODE_MAINGAME_SETTING,
 A27_SONGMODE_MAINGAME_WAITSTART,
 A27_SONGMODE_MAINGAME_START,
 A27_SONGMODE_MAINGAME_PROCESS,
 A27_SONGMODE_7,
 A27_SONGMODE_8,
 A27_SONGMODE_RESULT,
 A27_SONGMODE_10,
 A27_SONGMODE_RESULTDATA_SET,
 A27_SONGMODE_RESULTDATA_COMPLETE,
};

enum PM_Record_Mode{
    RECORD_MODE_DISABLED,
    RECORD_SONG_MODE,
    RECORD_PLAYBACK_MODE
};

// -- Internal Shared Structures --
#include "song_cursor.h"
#include "song_settings.h"
#include "song_state.h"
#include "song_event.h"
#include "song_judge.h"

typedef struct _CURSOR_TS{
    long long start;
    long long end;
}CursorTS,*PCursorTS;

typedef struct _CURSOR_TIMESTAMP{
    CursorTS cursor[PLAYER_CURSOR_MAX_ACTIVE];
}CursorTimestamp,*PCursorTimestamp;

typedef struct _CURSOR_TIMESTAMPS{
    CursorTimestamp player[2];
}CursorTimestamps,*PCursorTimestamps;

typedef struct _IO_TRACK_STATE{
    unsigned char track[8];
}IOTrackState,*PIOTrackState;

typedef struct _IO_TRACK_STATES{
    IOTrackState player[2];
}IOTrackStates,*PIOTrackStates;

// -- Internal Bindings --
#define TIMER_RESTART_DELAY 400
// Song Timer
long long SongTimer_GetSongElapsed(void);
long long SongTimer_GetSongStart(void);
short SongTimer_GetCurrentBeat(float ms_per_ebeat);
void SongTimer_Start(PSongSettings song_settings,PSongState state, PSongEvent event);
void SongTimer_Stop(void);
unsigned char SongTimer_IsRunning(void);
// Event Timer
PCursorTimestamps EventTimer_GetCursorTS(void);
void EventTimer_ClearCursor(PSongState song_state, unsigned char player_index, unsigned short cursor_index);
void EventTimer_Start(PSongSettings song_settings,PSongState state, PSongEvent event);
void EventTimer_Stop(void);
unsigned char EventTimer_IsRunning(void);
void EventTimer_AddToSoundEvents(unsigned short event_value);
// Scrolling Timer
void ScrollTimer_Start(PSongSettings song_settings,PSongState state, PSongEvent event);
void ScrollTimer_Stop(void);
unsigned char ScrollTimer_IsRunning(void);
// Judge Timer
void JudgeTimer_Start(PSongSettings song_settings,PSongState state, PSongEvent event, PSongJudge judge);
void JudgeTimer_Stop(void);
unsigned char JudgeTimer_IsRunning(void);
void JudgeTimer_Update(void);
// InputState Timer
void InputStateTimer_Start(PSongSettings song_settings,PSongState state, PSongEvent event);
void InputStateTimer_Stop(void);
unsigned char InputStateTimer_IsRunning(void);
void InputStateTimer_Update(void);
PIOTrackStates InputStateTimer_GetTrackState(void);
unsigned short InputStateTimer_GetKeySoundMapping(unsigned char track_index);

unsigned char SongManager_InSong(void);
void SongManager_StopSong(void);

// -- API --
unsigned int SongManager_Init(void* setting_data, void* response_buffer);
unsigned int SongManager_Reset(void* response_buffer);
unsigned int SongManager_Start(void* response_buffer);
unsigned int SongManager_Update(void* response_buffer);
void SongManager_Stop(void);
unsigned int SongManager_GetResult(void* response_buffer);
unsigned int SongManager_Record_Header(void* header_data, void* response_buffer);
unsigned int SongManager_Record_Body(void* body_data, void* response_buffer);
#endif
