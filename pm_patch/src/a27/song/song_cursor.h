#ifndef __SONG_CURSOR_H
#define __SONG_CURSOR_H

#define PLAYER_CURSOR_MAX_ACTIVE 150

typedef struct _CURSOR_STATE{
    unsigned short flags;
    unsigned char exflags;
    unsigned char fever_flags;
    short y_pos;
    short fever_offset;
}NoteCursor,*PNoteCursor;

#define ShowCursor(x) x.flags |=2
#define HideCursor(x) x.flags &= ~2
#define GetCursorFeverSize(x) x.y_pos + x.fever_offset

#define IsActivePCursor(x) (x->flags != 0)
#define IsActiveCursor(ts,pi,ci) (ts.player[pi].cursor[ci].end != 0)
#define IsHiddenCursor(x) ((x.flags & 2) == 0)
#define IsFeverCursor(x) ((x.fever_flags & 2) != 0)
#define GetFeverCombo(x) (x.fever_flags >> 2)
#define IsMeasureBarPCursor(x) (x->exflags != 0)
#define IsMeasureBarCursor(x) (x.exflags != 0)

#define SetMeasureCursor(x) x->flags |= 0x02; x->exflags |= 0x40; x->y_pos = 0
#define GetCursorTrack(x) (x.flags >> 6)
#define ClearCursor(x) x.exflags=0;x.flags=0;x.fever_flags=0;x.fever_offset=0;x.y_pos=0


typedef struct _PLAYER_CURSOR{
    NoteCursor cursor[PLAYER_CURSOR_MAX_ACTIVE];
}PlayerCursor,*PPlayerCursor;




#endif
