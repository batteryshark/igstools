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
static pthread_t trackinput_hthread;
static ThreadParams tp;

static unsigned char thread_running = 0;
unsigned char InputStateTimer_IsRunning(void){return thread_running;}

IOTrackStates track_state,last_sampled_track_state;

PIOTrackStates InputStateTimer_GetTrackState(void){
    return &track_state;
}

unsigned short InputStateTimer_GetKeySoundMapping(unsigned char track_index){
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

void InputStateTimer_Update(void){
            
        // Handle KeySounds and Lane Animation - This requires a per-frame debounce.
        for(int i=0;i<2;i++){
            if(!tp.state->player_isplaying[i]){continue;}
            for(int j=0;j<6;j++){                
                if(track_state.player[i].track[j] && !last_sampled_track_state.player[i].track[j]){
                    // If we're using the keysound for double hit, we have to disable the single hits. Otherwise, it sounds weird.
                    if(j == LANE_2DRUM){
                        tp.state->sound_index[(i*8) + LANE_DRUM] = KEYSOUND_OFF;
                    }
                    if(j == LANE_2RIM){
                        tp.state->sound_index[(i*8) + LANE_2RIM] = KEYSOUND_OFF;
                    }
                    tp.state->sound_index[(i*8) + j] = InputStateTimer_GetKeySoundMapping(j);    
                }
            }
            tp.state->player_track_hit_animation[i].track[LANE_BLUE] = track_state.player[i].track[LANE_BLUE] && !last_sampled_track_state.player[i].track[LANE_BLUE];
            tp.state->player_track_hit_animation[i].track[LANE_DRUM] = track_state.player[i].track[LANE_DRUM]&& !last_sampled_track_state.player[i].track[LANE_DRUM];
            tp.state->player_track_hit_animation[i].track[LANE_2DRUM] = track_state.player[i].track[LANE_2DRUM]&& !last_sampled_track_state.player[i].track[LANE_2DRUM];
            tp.state->player_track_hit_animation[i].track[LANE_RIM] = track_state.player[i].track[LANE_RIM]&& !last_sampled_track_state.player[i].track[LANE_RIM];
            tp.state->player_track_hit_animation[i].track[LANE_2RIM] = track_state.player[i].track[LANE_2RIM]&& !last_sampled_track_state.player[i].track[LANE_2RIM];
            tp.state->player_track_hit_animation[i].track[LANE_RED] = track_state.player[i].track[LANE_RED]&& !last_sampled_track_state.player[i].track[LANE_RED];
        }
        memcpy(&last_sampled_track_state,&track_state,sizeof(IOTrackStates));
}



static void* trackinput_thread(void* arg){
    PKeyIOState pio_state;
    thread_running = 1;
    while(SongManager_InSong()){
        pio_state = KeyIO_GetState();
        
        if(tp.state->player_isplaying[0]){
            track_state.player[0].track[LANE_BLUE] = pio_state->p1[KEYIO_PBLUE];
            track_state.player[0].track[LANE_RED] = pio_state->p1[KEYIO_PRED];
            track_state.player[0].track[LANE_DRUM] = pio_state->p1[KEYIO_PDRUM_L] || pio_state->p1[KEYIO_PDRUM_R];
            track_state.player[0].track[LANE_RIM] = pio_state->p1[KEYIO_PRIM_L] || pio_state->p1[KEYIO_PRIM_R];
            track_state.player[0].track[LANE_2DRUM] = pio_state->p1[KEYIO_PDRUM_L] && pio_state->p1[KEYIO_PDRUM_R];
            track_state.player[0].track[LANE_2RIM] = pio_state->p1[KEYIO_PRIM_L] && pio_state->p1[KEYIO_PRIM_R];
            if(track_state.player[0].track[LANE_2DRUM]){
                    track_state.player[0].track[LANE_DRUM] = 0;
            }
            if(track_state.player[0].track[LANE_2RIM]){
                    track_state.player[0].track[LANE_2RIM] = 0;
            }            
        }
        if(tp.state->player_isplaying[1]){
            track_state.player[1].track[LANE_BLUE] = pio_state->p2[KEYIO_PBLUE];
            track_state.player[1].track[LANE_RED] = pio_state->p2[KEYIO_PRED];
            track_state.player[1].track[LANE_DRUM] = pio_state->p2[KEYIO_PDRUM_L] || pio_state->p2[KEYIO_PDRUM_R];
            track_state.player[1].track[LANE_RIM] = pio_state->p2[KEYIO_PRIM_L] || pio_state->p2[KEYIO_PRIM_R];
            track_state.player[1].track[LANE_2DRUM] = pio_state->p2[KEYIO_PDRUM_L] && pio_state->p2[KEYIO_PDRUM_R];
            track_state.player[1].track[LANE_2RIM] = pio_state->p2[KEYIO_PRIM_L] && pio_state->p2[KEYIO_PRIM_R];
        }
            if(track_state.player[1].track[LANE_2DRUM]){
                    track_state.player[1].track[LANE_DRUM] = 0;
            }
            if(track_state.player[1].track[LANE_2RIM]){
                    track_state.player[1].track[LANE_2RIM] = 0;
            }          
    }
    thread_running = 0;

}


void InputStateTimer_Start(PSongSettings song_settings,PSongState state, PSongEvent event){ 
    if(thread_running){return;}
    tp.event = event;
    tp.settings = song_settings;
    tp.state = state;
    pthread_create(&trackinput_hthread, 0, trackinput_thread, NULL);
}
