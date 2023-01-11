#include <pthread.h>
#include <stdio.h>

#include "../../utils/utils.h"
#include "song_settings.h"
#include "song_state.h"
#include "song_event.h"
#include "song_manager.h"

typedef struct _THREAD_PARAMS{
    PSongSettings settings;
    PSongEvent event;
    PSongState state;
}ThreadParams,*PThreadParams;

static ThreadParams tp;

static pthread_t songtimer_hthread;
static long long song_start;
static long long song_elapsed;
static unsigned char thread_running = 0;
unsigned char SongTimer_IsRunning(void){return thread_running;}

long long SongTimer_GetSongStart(void){
    return song_start;
}

long long SongTimer_GetSongElapsed(void){
    return song_elapsed;
}

short SongTimer_GetCurrentBeat(float ms_per_ebeat){
    return (short)song_elapsed / ms_per_ebeat;
}

static void *song_timer_thread(void* arg){
    song_start = GetCurrentTimestamp();
    song_elapsed = 0;
    while(thread_running){                
        song_elapsed = GetCurrentTimestamp() - song_start;
        tp.state->current_beat[0] = (short)song_elapsed / tp.event->ms_per_ebeat;
        tp.state->current_beat[1] = (short)song_elapsed / tp.event->ms_per_ebeat;
    }
}


void SongTimer_Start(PSongSettings song_settings,PSongState state, PSongEvent event){
    if(thread_running){
        thread_running = 0;
        // Wait a bit for the original thread to die off.
        SleepMS(TIMER_RESTART_DELAY);
    }
    thread_running = 1;
    
    tp.event = event;
    tp.settings = song_settings;
    tp.state = state;
    pthread_create(&songtimer_hthread, 0, song_timer_thread, NULL);
}

void SongTimer_Stop(void){
    thread_running = 0;
}
