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

static void *scrolling_timer_thread(void* arg){
    long long cts;
    long long song_elapsed;
    PCursorTimestamps ts = EventTimer_GetCursorTS();
    while(thread_running){
            cts = GetCurrentTimestamp();
            song_elapsed = SongTimer_GetSongElapsed();
            for(int i=0;i<2;i++){
                if(!tp.state->player_isplaying[i]){continue;}
                for(int j=0;j<PLAYER_CURSOR_MAX_ACTIVE;j++){
                    // We only scroll visible stuff.
                    if(!IsHiddenCursor(tp.state->player_cursor[i].cursor[j])){                        
                        tp.state->player_cursor[i].cursor[j].y_pos = DeriveDistanceByTime(ts->player[i].cursor[j].start,song_elapsed, ts->player[i].cursor[j].end,0,JUDGE_CENTER);
                        // If it's a measure bar, we'll check if it's hit our beat zone and make it invisible.
                        if(IsMeasureBarCursor(tp.state->player_cursor[i].cursor[j])){
                            if(tp.state->player_cursor[i].cursor[j].y_pos > JUDGE_CENTER){
                                    EventTimer_ClearCursor(tp.state,i,j);
                            }
                        }
                        continue;
                    }else{
                        EventTimer_ClearCursor(tp.state,i,j);
                        continue;
                    }
                }               
            }
            SleepMS(1);
    }
}

void ScrollTimer_Start(PSongSettings song_settings,PSongState state, PSongEvent event){    
    if(thread_running){
        thread_running = 0;
        // Wait a bit for the original thread to die off.
        SleepMS(TIMER_RESTART_DELAY);
    }
    thread_running = 1;
    tp.event = event;
    tp.settings = song_settings;
    tp.state = state;
    pthread_create(&scrolltimer_hthread, 0, scrolling_timer_thread, NULL);
}

void ScrollTimer_Stop(void){
    thread_running = 0;
}
