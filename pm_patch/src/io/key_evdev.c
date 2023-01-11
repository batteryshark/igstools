#define _GNU_SOURCE
#include <stdio.h>
#include <linux/input.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#include "keyio.h"

static const char* default_event_device = "/dev/input/by-path/platform-i8042-serio-0-event-kbd";

int evdev_try_open(void){
    const char* selected_evdev = default_event_device;
    int kbd_evdev = -1;

    if(getenv("PM_KEYIO_EVDEV")){
        selected_evdev = getenv("PM_KEYIO_EVDEV");
    }
    kbd_evdev = open(selected_evdev,0);
    return kbd_evdev;
}

void evdev_input_loop(int kbd_evdev, PKeyIOState key_state, PKeyIOState last_key_state){
    printf("[KeyIO] Using evdev Event Loop\n");
    struct input_event ev;
    while(1){
        
        read(kbd_evdev, &ev, sizeof(ev));        
        if (ev.type == EV_KEY) {
            if(ev.code == KEY_ESC){ 
                printf("[A27_KeyIO] User Exited.\n");
                return;
            }
            memcpy(last_key_state,key_state,sizeof(KeyIOState));
            switch(ev.code){
                // System Controls
                case KEY_1:
                    key_state->system[KEYIO_STEST] = (ev.value) ? 1:0;
                    break;
                case KEY_2:
                    key_state->system[KEYIO_SSVC] = (ev.value) ? 1:0;
                    break;                    
                case KEY_3:
                    key_state->system[KEYIO_DEV_1] = (ev.value) ? 1:0;
                    break;
                case KEY_4:
                    key_state->system[KEYIO_DEV_2] = (ev.value) ? 1:0;
                    break;                 
                case KEY_5:
                    key_state->system[KEYIO_COIN] = (ev.value) ? 1:0;
                    break;                    
                // Player 1 Controls
                case KEY_Z:
                    key_state->p1[KEYIO_PRIM_L] = (ev.value) ? 1:0;
                    break; 
                case KEY_X: 
                    key_state->p1[KEYIO_PDRUM_L] = (ev.value) ? 1:0;
                    break;
                case KEY_C:
                    key_state->p1[KEYIO_PDRUM_R] = (ev.value) ? 1:0;
                    break;
                case KEY_V:
                    key_state->p1[KEYIO_PRIM_R] = (ev.value) ? 1:0;
                    break;
                case KEY_S:
                    key_state->p1[KEYIO_PBLUE] = (ev.value) ? 1:0;
                    break;                    
                case KEY_F: 
                    key_state->p1[KEYIO_PRED] = (ev.value) ? 1:0;
                    break;
                // Player 2 Controls
                case KEY_B:
                    key_state->p2[KEYIO_PRIM_L] = (ev.value) ? 1:0; 
                    break;
                case KEY_N:
                    key_state->p2[KEYIO_PDRUM_L] = (ev.value) ? 1:0; 
                    break;
                case KEY_M:
                    key_state->p2[KEYIO_PDRUM_R] = (ev.value) ? 1:0; 
                    break;
                case KEY_COMMA:
                    key_state->p2[KEYIO_PRIM_R] = (ev.value) ? 1:0; 
                    break;                    
                case KEY_H:
                    key_state->p2[KEYIO_PBLUE] = (ev.value) ? 1:0; 
                    break;
                case KEY_K:
                    key_state->p2[KEYIO_PRED] = (ev.value) ? 1:0; 
                    break;
                default:
                    break;
            }            
        }
    } 
}
