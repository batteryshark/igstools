#include <string.h>
#include <pthread.h>
#include <stdio.h>

#include "../../utils/utils.h"

#include "song_settings.h"
#include "song_state.h"
#include "song_event.h"
#include "song_manager.h"
#include "song_cursor.h"
#include "song_judge.h"

typedef struct _THREAD_PARAMS{
    PSongSettings settings;
    PSongEvent event;
    PSongState state;
}ThreadParams,*PThreadParams;


static unsigned char thread_running = 0;
unsigned char ScrollTimer_IsRunning(void){return thread_running;}
static pthread_t scrolltimer_hthread;
static ThreadParams tp;
#define SCROLL_MAX 0x200
float scroll_tail_speedmod[5] = {1,0.85,0.85,0.5,0.4};

static void *scrolling_timer_thread(void* arg){
    long long cts;
    long long song_elapsed;
    PCursorTimestamps ts = EventTimer_GetCursorTS();
    thread_running = 1;
    while(SongManager_InSong()){
            cts = GetCurrentTimestamp();
            song_elapsed = SongTimer_GetSongElapsed();
            for(int i=0;i<2;i++){
                if(!tp.state->player_isplaying[i]){continue;}
                tp.state->current_beat[i] = SongTimer_GetCurrentBeat(tp.event->ms_per_ebeat);
                float tail_scroll = tp.event->ms_per_beat * scroll_tail_speedmod[tp.settings->player_mod[i].speed];
                for(int j=0;j<PLAYER_CURSOR_MAX_ACTIVE;j++){
                    // We only scroll active stuff.
                    if(!IsActivePCursor(ts,i,j)){continue;}
                    
                    if(tp.state->player_cursor[i].cursor[j].y_pos > JUDGE_CENTER){
                        tp.state->player_cursor[i].cursor[j].y_pos = DeriveDistanceByTime(ts->player[i].cursor[j].start,song_elapsed, ts->player[i].cursor[j].end+(int)tail_scroll,0,SCROLL_MAX);
                    }else{
                        tp.state->player_cursor[i].cursor[j].y_pos = DeriveDistanceByTime(ts->player[i].cursor[j].start,song_elapsed, ts->player[i].cursor[j].end,0,JUDGE_CENTER);
                    }
                    
                    
                    if(IsMeasureBarCursor(tp.state->player_cursor[i].cursor[j])){
                        // Clear for Measure Bar Cursors
                        if(tp.state->player_cursor[i].cursor[j].y_pos >= JUDGE_CENTER){
                            EventTimer_ClearCursor(tp.state,i,j);
                        }  
                    }else{
                        // Clear For Normal Cursors and Fever Cursors
                        if(tp.state->player_cursor[i].cursor[j].y_pos+tp.state->player_cursor[i].cursor[j].fever_offset >= SCROLL_MAX){
                            EventTimer_ClearCursor(tp.state,i,j);
                        }                            
                    }
          
                }               
            }
            SleepMS(1);
    }
    thread_running = 0;
}

void ScrollTimer_Start(PSongSettings song_settings,PSongState state, PSongEvent event){    
    if(thread_running){return;}
    tp.event = event;
    tp.settings = song_settings;
    tp.state = state;
    pthread_create(&scrolltimer_hthread, 0, scrolling_timer_thread, NULL);
}
