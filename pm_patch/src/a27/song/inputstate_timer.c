#include <string.h>
#include <pthread.h>
#include <stdio.h>

#include "../../utils/utils.h"
#include "../../io/keyio.h"

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
static ThreadParams tp;

static unsigned char thread_running = 0;
unsigned char InputStateTimer_IsRunning(void){return thread_running;}


/*
IOTrackStates track_state;
IOTrackStates last_track_state;

static unsigned int last_last_swst = 0;
static unsigned int last_swst = 0;


void UpdateTrackInput(void){
    
    unsigned int swst = KeyIO_GetSwitches();
    memcpy(&last_track_state,&track_state,sizeof(IOTrackStates));
       
    if(state.player_isplaying[0]){
        track_state.player[0].track[LANE_BLUE] = IO_ISSSET(swst,last_swst,INP_P1_BLUE);
        track_state.player[0].track[LANE_DRUM] = IO_ISSSET(swst,last_swst,INP_P1_DRUM_L) || IO_ISSSET(swst,last_swst,INP_P1_DRUM_R);
        track_state.player[0].track[LANE_2DRUM] = (IO_ISSSET(swst,last_swst,INP_P1_DRUM_L) + IO_ISSSET(swst,last_swst,INP_P1_DRUM_R) + IO_ISSSET(last_swst,last_last_swst,INP_P1_DRUM_L) + IO_ISSSET(last_swst,last_last_swst,INP_P1_DRUM_R) > 1);
        track_state.player[0].track[LANE_RIM] = IO_ISSSET(swst,last_swst,INP_P1_RIM_L) || IO_ISSSET(swst,last_swst,INP_P1_RIM_R);
        track_state.player[0].track[LANE_2RIM] = (IO_ISSSET(swst,last_swst,INP_P1_RIM_L) + IO_ISSSET(swst,last_swst,INP_P1_RIM_R) + IO_ISSSET(last_swst,last_last_swst,INP_P1_RIM_L) + IO_ISSSET(last_swst,last_last_swst,INP_P1_RIM_R) > 1);
        track_state.player[0].track[LANE_RED] = IO_ISSSET(swst,last_swst,INP_P1_RED);
        
        // If we hit both on rim or drum, we have to flip the single lanes.
        if(track_state.player[0].track[LANE_2DRUM]){
            track_state.player[0].track[LANE_DRUM] = 0;
            last_track_state.player[0].track[LANE_2DRUM] = 1;            
        }
        if(track_state.player[0].track[LANE_2RIM]){
            track_state.player[0].track[LANE_RIM] = 0;
            last_track_state.player[0].track[LANE_2RIM] = 1;
        }
    }
    if(state.player_isplaying[1]){
        track_state.player[1].track[LANE_BLUE] = IO_ISSET(swst,INP_P2_BLUE);
        track_state.player[1].track[LANE_DRUM] = IO_ISSET(swst,INP_P2_DRUM_L) || IO_ISSET(swst,INP_P2_DRUM_R);
        track_state.player[1].track[LANE_2DRUM] = (IO_ISSET(swst,INP_P2_DRUM_L) + IO_ISSET(swst,INP_P2_DRUM_R) + IO_ISSET(last_swst,INP_P2_DRUM_L) + IO_ISSET(last_swst,INP_P2_DRUM_R) > 1);
        track_state.player[1].track[LANE_RIM] = IO_ISSET(swst,INP_P2_RIM_L) || IO_ISSET(swst,INP_P2_RIM_R);
        track_state.player[1].track[LANE_2RIM] =  (IO_ISSET(swst,INP_P2_RIM_L) + IO_ISSET(swst,INP_P2_RIM_R) + IO_ISSET(last_swst,INP_P2_RIM_L) + IO_ISSET(last_swst,INP_P2_RIM_R) > 1);
        track_state.player[1].track[LANE_RED] = IO_ISSET(swst,INP_P2_RED);
        
        // If we hit both on rim or drum, we have to flip the single lanes.
        if(track_state.player[1].track[LANE_2DRUM]){
            track_state.player[1].track[LANE_DRUM] = 0;
        }
        if(track_state.player[1].track[LANE_2RIM]){
            track_state.player[1].track[LANE_RIM] = 0;
        }
    }
    last_last_swst = last_swst;
    last_swst = swst; 
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


*/

unsigned short GetKeySoundMapping(unsigned char track_index){
    switch(track_index){
        case LANE_BLUE:
            return KEYSOUND_BLUE;
        case LANE_DRUM:
            return KEYSOUND_DRUM;
        case LANE_2DRUM:
            return KEYSOUND_2DRUM;
        case LANE_RIM:
            return KEYSOUND_RIM;
        case LANE_2RIM:
            return KEYSOUND_2RIM;
        case LANE_RED:
            return KEYSOUND_RED;
        default:
            return KEYSOUND_OFF;
    }
}

void InputStateTimer_Start(PSongSettings song_settings,PSongState state, PSongEvent event){
        if(thread_running){
        thread_running = 0;
        // Wait a bit for the original thread to die off.
       SleepMS(TIMER_RESTART_DELAY);
    }
    thread_running = 1;
    
    tp.event = event;
    tp.settings = song_settings;
    tp.state = state;
   // pthread_create(&songtimer_hthread, 0, song_timer_thread, NULL);
}

void InputStateTimer_Stop(void){
    thread_running = 0;
}
