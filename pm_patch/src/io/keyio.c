// KeyIO For PercussionMaster
#define _GNU_SOURCE
#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include "../a27/a27.h"
#include "../utils/utils.h"
#include "key_evdev.h"
#include "key_x11.h"

#include "keyio.h"


static pthread_t hthread;
static KeyIOState key_state,last_key_state;

static void *input_thread(void* args){
    int kbd_evdev = evdev_try_open();
    // Based on if we got an event device handle, use an evdev or X11 loop.
    (kbd_evdev == -1) ? x11_input_loop(&key_state,&last_key_state) : evdev_input_loop(kbd_evdev, &key_state, &last_key_state);
    QuitProcess();
}

PKeyIOState KeyIO_GetState(void){
    return &key_state;
}

void KeyIO_Init(void){
    memset(&key_state,0,sizeof(KeyIOState));
    memset(&last_key_state,0,sizeof(KeyIOState));
    pthread_create(&hthread, 0, input_thread, NULL);
}

// -- Exports --

unsigned int KeyIO_GetSwitchState(void){
    unsigned int n_state = 0;
    // Player 1
    if(key_state.p1[KEYIO_PDRUM_L] && !last_key_state.p1[KEYIO_PDRUM_L]){      IO_SET(n_state,INP_P1_DRUM_L);}
    if(key_state.p1[KEYIO_PDRUM_R] && !last_key_state.p1[KEYIO_PDRUM_R]){      IO_SET(n_state,INP_P1_DRUM_R);}
    if(key_state.p1[KEYIO_PRIM_L]  && !last_key_state.p1[KEYIO_PRIM_L]){       IO_SET(n_state,INP_P1_RIM_L);}
    if(key_state.p1[KEYIO_PRIM_R]  && !last_key_state.p1[KEYIO_PRIM_R]){       IO_SET(n_state,INP_P1_RIM_R);}
    if(key_state.p1[KEYIO_PBLUE]   && !last_key_state.p1[KEYIO_PBLUE]){        IO_SET(n_state,INP_P1_BLUE);}    
    if(key_state.p1[KEYIO_PRED]    && !last_key_state.p1[KEYIO_PRED]){         IO_SET(n_state,INP_P1_RED);}    
    
    // Player 2
    if(key_state.p2[KEYIO_PDRUM_L] && !last_key_state.p2[KEYIO_PDRUM_L]){      IO_SET(n_state,INP_P2_DRUM_L);}
    if(key_state.p2[KEYIO_PDRUM_R] && !last_key_state.p2[KEYIO_PDRUM_R]){      IO_SET(n_state,INP_P2_DRUM_R);}
    if(key_state.p2[KEYIO_PRIM_L]  && !last_key_state.p2[KEYIO_PRIM_L]){       IO_SET(n_state,INP_P2_RIM_L);}
    if(key_state.p2[KEYIO_PRIM_R]  && !last_key_state.p2[KEYIO_PRIM_R]){       IO_SET(n_state,INP_P2_RIM_R);}
    if(key_state.p2[KEYIO_PBLUE]   && !last_key_state.p2[KEYIO_PBLUE]){        IO_SET(n_state,INP_P2_BLUE);}    
    if(key_state.p2[KEYIO_PRED]    && !last_key_state.p2[KEYIO_PRED]){         IO_SET(n_state,INP_P2_RED);}     
    
    // System Switches
    if(key_state.system[KEYIO_STEST] && !last_key_state.system[KEYIO_STEST]){ IO_SET(n_state,INP_SW_TEST);}
    if(key_state.system[KEYIO_SSVC]  && !last_key_state.system[KEYIO_SSVC]){  IO_SET(n_state,INP_SW_SERVICE);}
    if(key_state.system[KEYIO_DEV_1] && !last_key_state.system[KEYIO_DEV_1]){ IO_SET(n_state,INP_DEV_1);}
    if(key_state.system[KEYIO_DEV_2] && !last_key_state.system[KEYIO_DEV_2]){ IO_SET(n_state,INP_DEV_2);}
    
    return n_state;
}

unsigned int KeyIO_GetCoinState(void){return (key_state.system[KEYIO_COIN] && !last_key_state.system[KEYIO_COIN]);}

void KeyIO_InjectWrite(PA27WriteHeader write_header){
    write_header->key_input = KeyIO_GetSwitchState();
}

void KeyIO_InjectRead(PA27ReadHeader read_header){
    read_header->button_io[0] = KeyIO_GetSwitchState();
    read_header->coin_inserted = KeyIO_GetCoinState();
    read_header->num_io_channels = 1;
}


