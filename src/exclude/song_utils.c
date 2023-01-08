#include <stdio.h>
#include <stdlib.h>

#include "song_defs.h"

#include "song_utils.h"


void ResetSoundEvents(void* state){
    PSongState song_state = state;
    for(int i=16;i<32;i++){
            song_state->sound_index[i] = 0;
    }
}

void AddToSoundEvents(void* state, unsigned short event_value){
    PSongState song_state = state;
    for(int i=16;i<32;i++){
        if(!song_state->sound_index[i]){
            song_state->sound_index[i] = event_value;   
        }
    }    
}

void SetKeySound(void*state, unsigned char slot, unsigned short value){
    PSongState song_state = state;
    song_state->sound_index[slot] = value;
}

void ClearKeySounds(void* state){
    PSongState song_state = state;
    for(int i=0;i<16;i++){
        song_state->sound_index[i] = 0;
    }
}

