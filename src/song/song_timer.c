// Timer-Related Functionality for Songs
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <time.h>


#include "song_settings.h"
#include "song_event.h"
#include "song_state.h"
#include "song_judge.h"
#include "song_cursor.h"

#include "../keyio.h"
#include "../utils.h"

#include "song_timer.h"


static short current_beat;
short GetCurrentBeat(void){return current_beat;}

static unsigned char in_song;
static pthread_t songtimer_hthread;
static pthread_t scrolling_hthread;
static pthread_t cursorevent_hthread;
static pthread_t trackinput_hthread;
static pthread_t soundevent_hthread;


static long long song_start;
static long long song_elapsed;

typedef struct _THREAD_PARAMS{
    PSongSettings settings;
    PSongEvent event;
    PSongState state;
    PIOTrackStates track_states;
}ThreadParams,*PThreadParams;

static CursorTimestamps ts;
static ThreadParams tp;

static void *song_timer_thread(void* arg){

    // While the song is playing, we'll determine the beat based on current time since song start.
    song_start = GetCurrentTimestamp();
    song_elapsed = 0;
    while(in_song){        
        // Then, basically we check the current elapsed time divided by number of ms per beat and we should get what beat we're on.
        song_elapsed = GetCurrentTimestamp() - song_start;
        current_beat = (int)(song_elapsed / tp.event->ms_per_ebeat);       
    }
}

static void* trackinput_thread(void* arg){

    unsigned int last_swst = 0;
    unsigned int swst = KeyIO_GetSwitches();
    while(in_song){  
        swst = KeyIO_GetSwitches();
        if(tp.state->player_isplaying[0]){
            tp.track_states->player[0].track[LANE_BLUE] = IO_ISSET(swst,INP_P1_BLUE);
            tp.track_states->player[0].track[LANE_DRUM] = IO_ISSET(swst,INP_P1_DRUM_L) || IO_ISSET(swst,INP_P1_DRUM_R);
            tp.track_states->player[0].track[LANE_2DRUM] = (IO_ISSET(swst,INP_P1_DRUM_L) + IO_ISSET(swst,INP_P1_DRUM_R) + IO_ISSET(last_swst,INP_P1_DRUM_L) + IO_ISSET(last_swst,INP_P1_DRUM_R) > 1);
            tp.track_states->player[0].track[LANE_RIM] = IO_ISSET(swst,INP_P1_RIM_L) || IO_ISSET(swst,INP_P1_RIM_R);
            tp.track_states->player[0].track[LANE_2RIM] = (IO_ISSET(swst,INP_P1_RIM_L) + IO_ISSET(swst,INP_P1_RIM_R) + IO_ISSET(last_swst,INP_P1_RIM_L) + IO_ISSET(last_swst,INP_P1_RIM_R) > 1);
            tp.track_states->player[0].track[LANE_RED] = IO_ISSET(swst,INP_P1_RED);
            
            // If we hit both on rim or drum, we have to flip the single lanes.
            if(tp.track_states->player[0].track[LANE_2DRUM]){
                tp.track_states->player[0].track[LANE_DRUM] = 0;
            }
            if(tp.track_states->player[0].track[LANE_2RIM]){
                tp.track_states->player[0].track[LANE_RIM] = 0;
            }
        }
        if(tp.state->player_isplaying[1]){
            tp.track_states->player[1].track[LANE_BLUE] = IO_ISSET(swst,INP_P2_BLUE);
            tp.track_states->player[1].track[LANE_DRUM] = IO_ISSET(swst,INP_P2_DRUM_L) || IO_ISSET(swst,INP_P2_DRUM_R);
            tp.track_states->player[1].track[LANE_2DRUM] = (IO_ISSET(swst,INP_P2_DRUM_L) + IO_ISSET(swst,INP_P2_DRUM_R) + IO_ISSET(last_swst,INP_P2_DRUM_L) + IO_ISSET(last_swst,INP_P2_DRUM_R) > 1);
            tp.track_states->player[1].track[LANE_RIM] = IO_ISSET(swst,INP_P2_RIM_L) || IO_ISSET(swst,INP_P2_RIM_R);
            tp.track_states->player[1].track[LANE_2RIM] =  (IO_ISSET(swst,INP_P2_RIM_L) + IO_ISSET(swst,INP_P2_RIM_R) + IO_ISSET(last_swst,INP_P2_RIM_L) + IO_ISSET(last_swst,INP_P2_RIM_R) > 1);
            tp.track_states->player[1].track[LANE_RED] = IO_ISSET(swst,INP_P2_RED);
            
            // If we hit both on rim or drum, we have to flip the single lanes.
            if(tp.track_states->player[1].track[LANE_2DRUM]){
                tp.track_states->player[1].track[LANE_DRUM] = 0;
            }
            if(tp.track_states->player[1].track[LANE_2RIM]){
                tp.track_states->player[1].track[LANE_RIM] = 0;
            }
        }        
        last_swst = swst;
    }
}

void AddToSoundEvents(unsigned short event_value){
    for(int i=15;i<32;i++){
        if(!tp.state->sound_index[i]){
            tp.state->sound_index[i] = event_value;   
        }
    }    
}

static void *soundevent_thread(void* arg){       
   
    while(in_song){        
    
        for(int i=0; i < tp.event->num_sound_events; i++){
            PSoundEvent ce = &tp.event->sound_event[i];                
            if(ce->spawn_ms > 0 && ce->spawn_ms <= song_elapsed){
                AddToSoundEvents(ce->event_value);
                ce->spawn_ms = -1;
            }
        }            

        msleep(1);
    }
}

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
        if(!IsActivePCursor(target)){
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
        if(!IsActivePCursor(target)){
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


static void *cursorevent_thread(void* arg){

    while(in_song){             

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

        msleep(1);
    }
}


unsigned short derive_cursor_timescale(long start_time, long end_time, unsigned int speed_mod){
    

    
    // Start Time in MS in the song of the note - it's the spawn time.
    // the end time which is arguably more important beacuse it's when you have to be at JUDGE_CENTER
    // song_elapsed - start_time tells you how long the  note has existed
    // end_time - song_elapsed tells you how much time there is left before it has to be there
    // end_time - start_time tells you how long it will exist onscreen (probably where you get your multiple of speed mod 
    long cursor_lifetime = end_time - start_time;
    // so in cursor lifetime, we have to go from 0->0x179
    long cursor_timeleft = end_time - song_elapsed;
    long cursor_progress = cursor_lifetime - cursor_timeleft;
    float cursor_progress_ratio = (float)cursor_progress/(float)cursor_lifetime;
    float fdistance = (float)JUDGE_CENTER * cursor_progress_ratio;
    
    return (unsigned short)fdistance;
}

static void *scrolling_timer_thread(void* arg){
    long long cts;
    while(in_song){
            cts = GetCurrentTimestamp();
            for(int i=0;i<2;i++){
                if(!tp.state->player_isplaying[i]){continue;}
                for(int j=0;j<PLAYER_CURSOR_MAX_ACTIVE;j++){
                    // We only scroll visible stuff.
                    if(!IsHiddenCursor(tp.state->player_cursor[i].cursor[j])){                        
                        tp.state->player_cursor[i].cursor[j].y_pos = derive_cursor_timescale(ts.player[i].cursor[j].start,ts.player[i].cursor[j].end,tp.settings->player_mod[i].speed + 1);
                        // If it's a measure bar, we'll check if it's hit our beat zone and make it invisible.
                        if(IsMeasureBarCursor(tp.state->player_cursor[i].cursor[j])){
                            if(tp.state->player_cursor[i].cursor[j].y_pos >= JUDGE_CENTER){
                                    ClearCursor(tp.state->player_cursor[i].cursor[j]);
                            }
                        }
                        continue;
                    }else{
                        ClearCursor(tp.state->player_cursor[i].cursor[j]);
                        continue;
                    }
                }                
            }
            msleep(1);
    }
}


// -- Interface

void StopSongThreads(void){
    in_song = 0;
}

void StartSongThreads(PSongSettings song_settings,PSongState state, PSongEvent event, PIOTrackStates track_states){
    if(in_song){
        StopSongThreads();
    }    
    current_beat = -1;
    in_song = 1;
    tp.event = event;
    tp.settings = song_settings;
    tp.state = state;
    tp.track_states = track_states;
    
    pthread_create(&songtimer_hthread, 0, song_timer_thread, NULL);
    pthread_create(&scrolling_hthread,0,scrolling_timer_thread,NULL);
    pthread_create(&cursorevent_hthread,0,cursorevent_thread,NULL);
    pthread_create(&soundevent_hthread,0,soundevent_thread,NULL);
  //  pthread_create(&trackinput_hthread,0,trackinput_thread,NULL);
    
}

