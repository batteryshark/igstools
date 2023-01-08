#ifndef __SONG_TIMER_H
#define __SONG_TIMER_H


typedef struct _IO_TRACK_STATE{
    unsigned char track[8];
}IOTrackState,*PIOTrackState;

typedef struct _IO_TRACK_STATES{
    IOTrackState player[2];
}IOTrackStates,*PIOTrackStates;

typedef struct _CURSOR_TS{
    long start;
    long end;
}CursorTS,*PCursorTS;

typedef struct _CURSOR_TIMESTAMP{
    CursorTS cursor[PLAYER_CURSOR_MAX_ACTIVE];
}CursorTimestamp,*PCursorTimestamp;

typedef struct _CURSOR_TIMESTAMPS{
    CursorTimestamp player[2];
}CursorTimestamps,*PCursorTimestamps;

short GetCurrentBeat(void);
void StartSongThreads(PSongSettings song_settings, PSongState state, PSongEvent event, PIOTrackStates track_states);
void StopSongThreads(void);

#endif
