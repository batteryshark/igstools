#include <string.h>
#include <pthread.h>
#include <stdio.h>

#include "../../utils/utils.h"

#include "song_settings.h"
#include "song_state.h"
#include "song_event.h"
#include "song_manager.h"
#include "song_cursor.h"

typedef struct _THREAD_PARAMS{
    PSongSettings settings;
    PSongEvent event;
    PSongState state;
}ThreadParams,*PThreadParams;


static unsigned char thread_running = 0;

unsigned char EventTimer_IsRunning(void){return thread_running;}
static pthread_t eventtimer_hthread;
static ThreadParams tp;
static CursorTimestamps ts;


// We'll add a measure bar either to a free slot, or a slot we've already occupied as a measure bar.
void AddMeasureBar(PSongState song_state, unsigned char player_index, PMeasureEvent measure){
    PNoteCursor target;
   
    // Attempt 1 - Fill a measure bar slot that already exists.
    for(int i=0;i<PLAYER_CURSOR_MAX_ACTIVE;i++){
            target = &song_state->player_cursor[player_index].cursor[i];
            if(IsMeasureBarPCursor(target)){
                SetMeasureCursor(target);
                
                ts.player[player_index].cursor[i].start = measure->spawn_ms;
                ts.player[player_index].cursor[i].end = measure->event_ms;
                return;
            }
    }
    
    
    // Attempt 2 - Fill a new slot
    for(int i=0;i<PLAYER_CURSOR_MAX_ACTIVE;i++){
        target = &song_state->player_cursor[player_index].cursor[i];        
        if(!IsActiveCursor(ts,player_index,i)){
                SetMeasureCursor(target);
                ts.player[player_index].cursor[i].start = measure->spawn_ms;
                ts.player[player_index].cursor[i].end = measure->event_ms;
                return;
        }
    }
}

void AddCursor(PSongState song_state, unsigned char player_index, PCursorEvent cursor){
    PNoteCursor target;    
    // Attempt 1 - Fill a new slot
    for(int i=0;i<PLAYER_CURSOR_MAX_ACTIVE;i++){
        target = &song_state->player_cursor[player_index].cursor[i];
        if(!IsActiveCursor(ts,player_index,i)){
                target->flags = cursor->flags;
                target->exflags = 0;
                target->fever_flags = cursor->fever_flag;
                target->fever_offset = cursor->fever_offset;
                target->y_pos = 0;
                ts.player[player_index].cursor[i].start = cursor->spawn_ms;
                ts.player[player_index].cursor[i].end = cursor->event_ms;
                return;
        }
    }
}

void EventTimer_ClearCursor(PSongState song_state, unsigned char player_index, unsigned short cursor_index){
    ts.player[player_index].cursor[cursor_index].end = 0;
    ts.player[player_index].cursor[cursor_index].start = 0;
    song_state->player_cursor[player_index].cursor[cursor_index].flags = 0;
    song_state->player_cursor[player_index].cursor[cursor_index].exflags = 0;
    song_state->player_cursor[player_index].cursor[cursor_index].fever_flags = 0;
    song_state->player_cursor[player_index].cursor[cursor_index].y_pos = 0;
    song_state->player_cursor[player_index].cursor[cursor_index].fever_offset = 0;
}

void EventTimer_AddToSoundEvents(unsigned short event_value){
    for(int i=16;i<32;i++){
        if(!tp.state->sound_index[i]){
           tp.state->sound_index[i] = event_value;   
        }
    }    
}


void EventTimer_ClearSoundEvents(void){
    for(int i=0;i<32;i++){
        if(!tp.state->sound_index[i]){
           tp.state->sound_index[i] = 0;   
        }        
    }       
}

static void *event_thread(void* arg){
    thread_running = 1;
    while(SongManager_InSong()){             
        long long song_elapsed = SongTimer_GetSongElapsed();
        for(int i=0;i<2;i++){
            // So what we need to do here is - every beat, look if we're in a new measure of the song
            // if we are, spawn one and give it a hit date of the next ms offset of the next measure
            
            if(!tp.state->player_isplaying[i]){continue;}
            
            for(int j=0;j<tp.event->num_measure_events;j++){
                    if(tp.event->measure_events[i].measure[j].spawn_ms <= song_elapsed && tp.event->measure_events[i].measure[j].spawn_ms > 0){
                        AddMeasureBar(tp.state,i,&tp.event->measure_events[i].measure[j]);
                        tp.event->measure_events[i].measure[j].spawn_ms = -1;
                    }
            }            
            unsigned short num_cursor_events = (i) ? tp.event->p2_num_cursor_events : tp.event->p1_num_cursor_events;
            PCursorEvent player_cursor_events = (i) ? tp.event->p2_event : tp.event->p1_event;
            for(int j=0;j<num_cursor_events;j++){
                if(player_cursor_events[j].spawn_ms <= song_elapsed && player_cursor_events[j].spawn_ms > 0){
                    AddCursor(tp.state, i,&player_cursor_events[j]);
                    // We'll set this to inactive essentially so we don't use it again.
                    player_cursor_events[j].spawn_ms = -1;
                }
            }
        }           

        SleepMS(1);
    }
    thread_running = 0;
}

PCursorTimestamps EventTimer_GetCursorTS(void){
    return &ts;
}

void EventTimer_Start(PSongSettings song_settings,PSongState state, PSongEvent event){
    if(thread_running){return;}
    memset(&ts,0,sizeof(CursorTimestamps));
    tp.event = event;
    tp.settings = song_settings;
    tp.state = state;
    pthread_create(&eventtimer_hthread, 0, event_thread, NULL);
}
