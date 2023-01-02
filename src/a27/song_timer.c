#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <time.h>

#include "../utils.h"
#include "song_defs.h"
#include "song_utils.h"
#include "song_timer.h"

static short current_beat;
static unsigned char in_song;
static pthread_t songtimer_hthread;
static pthread_t soundevent_hthread;
static pthread_t scrolling_hthread;
static pthread_t cursorevent_hthread;
static long ms_per_measure;

static long long p1_cursor_ts[150];
static long long p2_cursor_ts[150];

static long long song_start;
static long long song_elapsed;

void msleep(unsigned int num_ms){
    struct timespec ts;
    ts.tv_sec = num_ms / 1000;
    ts.tv_nsec = (num_ms % 1000) * 1000000;
    nanosleep(&ts,NULL);
    
}

static void *song_timer_thread(void* arg){
    PSongParams parms = (PSongParams)arg;
    
    float song_tempo = parms->song_info->header.tempo;
    // First - Given Beats Per Minute, calculate how many ms per beat. (quarter note)
    float ms_per_beat = (float)((60 * 1000) / song_tempo);
    ms_per_measure = ms_per_beat * 4;
    // Next - Divide ms per measure by 8 to get the ms per eighth of a beat (32nd note)
    unsigned int ms_per_ebeat = (int)(ms_per_beat / 8);


    // While the song is playing, we'll determine the beat based on current time since song start.
    song_start = GetCurrentTimestamp();
    song_elapsed = 0;
    while(in_song){        
        // Then, basically we check the current elapsed time divided by number of ms per beat and we should get what beat we're on.
        song_elapsed = GetCurrentTimestamp() - song_start;
        current_beat = (int)(song_elapsed / ms_per_ebeat);       
    }
}

static void *soundevent_thread(void* arg){
    PSongParams parms = (PSongParams)arg;    
    unsigned short last_beat = current_beat;
    
    while(in_song){        
        if(current_beat != last_beat){            
            for(int i=0; i < parms->song_info->header.total_soundevents; i++){
                PSoundEvent ce = parms->song_info->sound_events + i;                
                if(ce->event_beat == current_beat){
                    AddToSoundEvents(parms->song_state,ce->event_value);
                    ce->event_beat = -1;
                }
            }            
            last_beat = current_beat;            
        }
        
    }
}

void AddToCursors(PSongState song_state,PCursorEvent ce){
    PNoteCursor target;
    if(song_state->p1_playing){
        for(int i=0;i<150;i++){
            target = &song_state->p1_cursor[i];
            if(!target->cursor_flags){
                target->cursor_flags = ce->flags;
                target->cursor_ypos = 0;
                target->cursor_exflags = ce->ex_flag;
                target->cursor_stretch = ce->cursor_swoff;
                target->cursor_holdflags = ce->hold_flag;
                p1_cursor_ts[i] = GetCurrentTimestamp();
                break;
            }
        }
    }
    if(song_state->p2_playing){
        for(int i=0;i<150;i++){
            target = &song_state->p2_cursor[i];
            if(!target->cursor_flags){
                target->cursor_flags = ce->flags;
                target->cursor_ypos = 0;
                target->cursor_exflags = ce->ex_flag;
                target->cursor_stretch = ce->cursor_swoff;
                target->cursor_holdflags = ce->hold_flag;
                p2_cursor_ts[i] = GetCurrentTimestamp();
                break;
            }
        }        
    }
    
    ce->event_beat = -1;
}

void AddMeasureBar(PSongState song_state){
    PNoteCursor target;
    if(song_state->p1_playing){
        for(int i=0;i<150;i++){
            target = &song_state->p1_cursor[i];
            if(!target->cursor_flags || (target->cursor_exflags && (target->cursor_flags & 2) == 0)){
                target->cursor_flags |= 0x02;
                target->cursor_exflags = 0x40;
                target->cursor_ypos = 0;
                p1_cursor_ts[i] = GetCurrentTimestamp();
                break;
            }
        }        
    }
    if(song_state->p2_playing){
        for(int i=0;i<150;i++){
            target = &song_state->p2_cursor[i];
            if(!target->cursor_flags || (target->cursor_exflags && (target->cursor_flags & 2) == 0)){
                target->cursor_flags |= 0x02;
                target->cursor_exflags = 0x40;
                target->cursor_ypos = 0;
                p2_cursor_ts[i] = GetCurrentTimestamp();
                break;
            }
            
        }        
    }    
}

static void *cursorevent_thread(void* arg){
    PSongParams parms = (PSongParams)arg;    
    unsigned short last_beat = current_beat;
    short spawn_beat;
    //AddMeasureBar(parms->song_state);
    while(in_song){             
        if(current_beat != last_beat){
            if(current_beat % 32 == 0){
                AddMeasureBar(parms->song_state);    
            }
            
            for(int i=0; i < parms->song_info->header.total_cursorevents; i++){
                PCursorEvent ce = parms->song_info->cursor_events + i;
                // We have to calculate a beat to spawn at... I'll try a static value first.
                if(ce->event_beat == -1){continue;}
                spawn_beat = ce->event_beat - 32;
                
                if(spawn_beat <= current_beat){    
                    AddToCursors(parms->song_state,ce);
                }
            }            
            last_beat = current_beat;            
        }
        
    }
}

void ClearCursor(PNoteCursor ccur){
    ccur->cursor_exflags = 0;
    ccur->cursor_flags = 0;
    ccur->cursor_holdflags = 0;
    ccur->cursor_stretch = 0;
    ccur->cursor_ypos = 0;    
}

unsigned short derive_cursor_timescale(long long cts, long long spawn_time){
    long long elapsed_time = cts-spawn_time;
    float progress_ratio = (float)elapsed_time / (float)ms_per_measure;
    float distance_value = (float)JUDGE_CENTER * progress_ratio;
    return (unsigned short)distance_value;
}

static void *scrolling_timer_thread(void* arg){
    PSongParams parms = (PSongParams)arg;
    PNoteCursor ccur;    
    long long cts;
    while(in_song){
            cts = GetCurrentTimestamp();
            if(parms->song_state->p1_playing){
                for(int i=0;i<150;i++){
                    ccur = &parms->song_state->p1_cursor[i];
                    // We only scroll visible stuff
                    if((ccur->cursor_flags & 2) > 0){
                        ccur->cursor_ypos = derive_cursor_timescale(cts,p1_cursor_ts[i]);
                        
                        // If it's a measure bar, we'll check if it's hit our beat zone and make it invisible.
                        if(ccur->cursor_exflags){
                            if(ccur->cursor_ypos >= JUDGE_CENTER){
                                ccur->cursor_flags &= ~2;
                            }
                        }
                        continue;
                    // If it's an invisible cursor, we'll clear it
                    }else if((ccur->cursor_flags != 0)){
                        ClearCursor(ccur);
                        continue;
                
                    }
                }
            }
            
            msleep(1);
        }
        
}


short GetCurrentBeat(void){return current_beat;}


void StopTimer(void){
    in_song = 0;
}

void StartTimer(void* parms){
    if(in_song){StopTimer();}
    current_beat = -1;
    in_song = 1;
    pthread_create(&songtimer_hthread, 0, song_timer_thread, parms);
    pthread_create(&soundevent_hthread,0,soundevent_thread,parms);
    pthread_create(&cursorevent_hthread,0,cursorevent_thread,parms);
    pthread_create(&scrolling_hthread,0,scrolling_timer_thread,parms);
    
}
