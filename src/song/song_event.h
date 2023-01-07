#ifndef __SONG_EVENT_H
#define __SONG_EVENT_H

#include "song_recfile.h"

#define MAX_REC_NOTES 999

typedef struct _SoundEvent{
    short spawn_beat;
    short event_beat;
    unsigned short event_value;
}SoundEvent,*PSoundEvent;

typedef struct _CursorEvent{
    long long spawn_timestamp;
    short event_beat;
    short spawn_beat;    
    unsigned short flags;
    unsigned char fever_flag;
    short fever_offset;
}CursorEvent,*PCursorEvent;

typedef struct _SONG_EVENT{
    float tempo;    
    unsigned int chart_id;
    unsigned short num_beats;
    unsigned short total_notes;
    unsigned short min_notes;
    unsigned char scroll_velocity;
    unsigned short p1_num_cursor_events;
    unsigned short p2_num_cursor_events;
    unsigned short num_sound_events;
    CursorEvent p1_event[MAX_REC_NOTES];
    CursorEvent p2_event[MAX_REC_NOTES];
    SoundEvent  sound_event[MAX_REC_NOTES];
}SongEvent,*PSongEvent;


void ParseRecHeader(PRecFile rec_file, PSongEvent song_event);
unsigned int ParseCursorEvents(PRecFileLane* plane,PCursorEvent* cursor_events, float tempo, unsigned char speed_mod);
unsigned int ParseSoundEvents(PRecFileLane* plane, PSoundEvent* sound_events, float tempo);

#endif
