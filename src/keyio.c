// KeyIO For PercussionMaster
#define _GNU_SOURCE
#include <stdio.h>
#include <linux/input.h>
#include <pthread.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

// X11 includes
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <X11/Xatom.h>
#include <X11/keysym.h>

#include "a27/a27.h"
#include "utils.h"

#include "keyio.h"

static const char* default_event_device = "/dev/input/by-path/platform-i8042-serio-0-event-kbd";
static pthread_t hthread;
static IOTrackStates track_states;

PIOTrackStates GetIOTrackStates(void){
    return &track_states;
}


static struct iostate keystate;
static struct iostate last_keystate;


unsigned int KeyIO_GetSwitches(void){
    unsigned int n_state = 0;
    
    // Player 1
    if(keystate.p1.drum_l && !last_keystate.p1.drum_l){IO_SET(n_state,INP_P1_DRUM_L);}
    if(keystate.p1.drum_r && !last_keystate.p1.drum_r){IO_SET(n_state,INP_P1_DRUM_R);}
    if(keystate.p1.rim_r && !last_keystate.p1.rim_r) {IO_SET(n_state,INP_P1_RIM_R) ;}
    if(keystate.p1.blue && !last_keystate.p1.blue)  {IO_SET(n_state,INP_P1_BLUE)  ;}
    if(keystate.p1.red && !last_keystate.p1.red)   {IO_SET(n_state,INP_P1_RED)   ;}
    if(keystate.p1.rim_l && !last_keystate.p1.rim_l) {IO_SET(n_state,INP_P1_RIM_L) ;}

    // Player 2
    if(keystate.p2.blue  && !last_keystate.p2.blue)  {IO_SET(n_state,INP_P2_BLUE)  ;}
    if(keystate.p2.red && !last_keystate.p2.red)   {IO_SET(n_state,INP_P2_RED)   ;}
    if(keystate.p2.rim_l && !last_keystate.p2.rim_l) {IO_SET(n_state,INP_P2_RIM_L) ;}
    if(keystate.p2.drum_l && !last_keystate.p2.drum_l){IO_SET(n_state,INP_P2_DRUM_L);}
    if(keystate.p2.drum_r && !last_keystate.p2.drum_r){IO_SET(n_state,INP_P2_DRUM_R);}
    if(keystate.p2.rim_r && !last_keystate.p2.rim_r) {IO_SET(n_state,INP_P2_RIM_R) ;}

    // System Switches
    if(keystate.sw_service) {IO_SET(n_state,INP_SW_SERVICE);}
    if(keystate.sw_test)    {IO_SET(n_state,INP_SW_TEST)   ;}
    
    // Dev Switches
    if(keystate.hidden_sw[0]){IO_SET(n_state,INP_DEV_1);}
    if(keystate.hidden_sw[1]){IO_SET(n_state,INP_DEV_2);}    
    
    //
    
    return n_state;
}

struct iostate* KeyIO_GetState(void){
 return &keystate;   
}

void evdev_input_loop(int kbd_evdev){
    printf("[KeyIO] Using evdev Event Loop\n");
    struct input_event ev;
    while(1){
        read(kbd_evdev, &ev, sizeof(ev));        
        if (ev.type == EV_KEY) {
            if(ev.code == KEY_ESC){ 
                printf("[A27_KeyIO] User Exited.\n");
                Shutdown();
            }
            memcpy(&last_keystate,&keystate,sizeof(struct iostate));
            switch(ev.code){
                // System Controls
                case KEY_1:
                    keystate.sw_test = (ev.value) ? 1:0;
                    break;
                case KEY_2:
                    keystate.sw_service = (ev.value) ? 1:0;
                    break;                    
                case KEY_3:
                    keystate.hidden_sw[0] = (ev.value) ? 1:0;
                    break;
                case KEY_4:
                    keystate.hidden_sw[1] = (ev.value) ? 1:0;
                    break;                 
                case KEY_5:
                    keystate.coin = (ev.value) ? 1:0;
                    break;                    
                // Player 1 Controls
                case KEY_Z:
                    keystate.p1.rim_l = (ev.value) ? 1:0;
                    break; 
                case KEY_X: 
                    keystate.p1.drum_l = (ev.value) ? 1:0;
                    break;
                case KEY_C:
                    keystate.p1.drum_r = (ev.value) ? 1:0;
                    break;
                case KEY_V:
                    keystate.p1.rim_r = (ev.value) ? 1:0;
                    break;
                case KEY_S:
                    keystate.p1.blue = (ev.value) ? 1:0;
                    break;                    
                case KEY_F: 
                    keystate.p1.red = (ev.value) ? 1:0;
                    break;
                // Player 2 Controls
                case KEY_B:
                    keystate.p2.rim_l = (ev.value) ? 1:0; 
                    break;
                case KEY_N:
                    keystate.p2.drum_l = (ev.value) ? 1:0; 
                    break;
                case KEY_M:
                    keystate.p2.drum_r = (ev.value) ? 1:0; 
                    break;
                case KEY_COMMA:
                    keystate.p2.rim_r = (ev.value) ? 1:0; 
                    break;                    
                case KEY_H:
                    keystate.p2.blue = (ev.value) ? 1:0; 
                    break;
                case KEY_K:
                    keystate.p2.red = (ev.value) ? 1:0; 
                    break;
                default:
                    break;
            }            
        }
    } 
}

void x11_input_loop(void){
    printf("[KeyIO] Using X11 Event Loop\n");
    int ksym;
    Display *display;
    XEvent event;

    display = XOpenDisplay(NULL);
    if (display == NULL){
        printf("Cannot open display\n");
        exit(-1);
    }
    XGrabKeyboard(display, DefaultRootWindow(display),
                 True, GrabModeAsync, GrabModeAsync, CurrentTime);
    while (1){
        memcpy(&last_keystate,&keystate,sizeof(struct iostate));
        XNextEvent(display, &event);
        if (event.type == KeyPress || event.type == KeyRelease){
            ksym = XLookupKeysym(&event.xkey, 0);         
            if(ksym == XK_Escape){
                printf("[A27_KeyIO] User Exited.\n");               
                Shutdown();
            }                
            switch(ksym){            
                // System Switches
                case XK_1:
                    keystate.sw_test = (event.type == KeyPress) ? 1 : 0;
                    break;
                case XK_2:
                    keystate.sw_service = (event.type == KeyPress) ? 1 : 0;
                    break;
                case XK_3:
                    keystate.hidden_sw[0] = (event.type == KeyPress) ? 1:0;
                    break;
                case XK_4:
                    keystate.hidden_sw[1] = (event.type == KeyPress) ? 1:0;
                    break;
                case XK_5:
                    keystate.coin = (event.type == KeyPress) ? 1 : 0;
                    break;                  
                // Player 1 
                case XK_z:
                    keystate.p1.rim_l = (event.type == KeyPress) ? 1 : 0;
                    break;
                case XK_x:
                    keystate.p1.drum_l = (event.type == KeyPress) ? 1 : 0;
                    break;
                case XK_c:
                    keystate.p1.drum_r = (event.type == KeyPress) ? 1 : 0;
                    break;
                case XK_v:
                    keystate.p1.rim_r = (event.type == KeyPress) ? 1 : 0;
                    break;
                case XK_s:
                    keystate.p1.blue = (event.type == KeyPress) ? 1 : 0;
                    break;
                case XK_f:
                    keystate.p1.red = (event.type == KeyPress) ? 1 : 0;
                    break;
                // Player 2 
                case XK_b:
                    keystate.p2.rim_l = (event.type == KeyPress) ? 1 : 0;
                    break;
                case XK_n:
                    keystate.p2.drum_l = (event.type == KeyPress) ? 1 : 0;
                    break;
                case XK_m:
                    keystate.p2.drum_r = (event.type == KeyPress) ? 1 : 0;
                    break;
                case XK_comma:
                    keystate.p2.rim_r = (event.type == KeyPress) ? 1 : 0;
                    break;
                case XK_h:
                    keystate.p2.blue = (event.type == KeyPress) ? 1 : 0;
                    break;
                case XK_k:
                    keystate.p2.red = (event.type == KeyPress) ? 1 : 0;
                    break;                                                                                               
                default:
                break;
            }            
        }
    }
}

static void *input_thread(void *arg){
    const char* selected_evdev = default_event_device;
    int kbd_evdev = -1;

    if(getenv("PM_KEYIO_EVDEV")){
        selected_evdev = getenv("PM_KEYIO_EVDEV");
    }
    kbd_evdev = open(selected_evdev,0);
    
    // Based on if we got an event device handle, use an evdev or X11 loop.
    (kbd_evdev == -1) ? x11_input_loop() : evdev_input_loop(kbd_evdev);
}

// -- Exports --



// This version does not respect debounce.
unsigned int KeyIO_GetSwitchesCurrent(void){
    unsigned int n_state = 0;
    
    // Player 1
    if(keystate.p1.drum_l ){IO_SET(n_state,INP_P1_DRUM_L);}
    if(keystate.p1.drum_r ){IO_SET(n_state,INP_P1_DRUM_R);}
    if(keystate.p1.rim_r ) {IO_SET(n_state,INP_P1_RIM_R) ;}
    if(keystate.p1.blue )  {IO_SET(n_state,INP_P1_BLUE)  ;}
    if(keystate.p1.red )   {IO_SET(n_state,INP_P1_RED)   ;}
    if(keystate.p1.rim_l ) {IO_SET(n_state,INP_P1_RIM_L) ;}

    // Player 2
    if(keystate.p2.blue  )  {IO_SET(n_state,INP_P2_BLUE)  ;}
    if(keystate.p2.red )   {IO_SET(n_state,INP_P2_RED)   ;}
    if(keystate.p2.rim_l ) {IO_SET(n_state,INP_P2_RIM_L) ;}
    if(keystate.p2.drum_l ){IO_SET(n_state,INP_P2_DRUM_L);}
    if(keystate.p2.drum_r ){IO_SET(n_state,INP_P2_DRUM_R);}
    if(keystate.p2.rim_r ) {IO_SET(n_state,INP_P2_RIM_R) ;}

    // System Switches
    if(keystate.sw_service) {IO_SET(n_state,INP_SW_SERVICE);}
    if(keystate.sw_test)    {IO_SET(n_state,INP_SW_TEST)   ;}
    
    // Dev Switches
    if(keystate.hidden_sw[0]){IO_SET(n_state,INP_DEV_1);}
    if(keystate.hidden_sw[1]){IO_SET(n_state,INP_DEV_2);}    
    return n_state;
}


unsigned int KeyIO_GetCoinState(void){
    return keystate.coin;
}

void KeyIO_InjectWrite(unsigned char* buf){
    struct A27_Write_Message* msg = (struct A27_Write_Message*)buf;
    msg->key_input = KeyIO_GetSwitches();
    A27SetWriteChecksum(msg);
}

void KeyIO_InjectRead(unsigned char* buf){
    unsigned int swst;
	struct A27_Read_Message* msg = (struct A27_Read_Message*)buf;
	
    swst = KeyIO_GetSwitches();

    msg->coin_inserted = KeyIO_GetCoinState();
    
	//msg->num_io_channels = 6;
	// Try this first.
	msg->num_io_channels = 1;
	msg->button_io[0] = swst;
	/*
    msg->button_io[2] = swst;
    msg->button_io[3] = swst;
    msg->button_io[4] = swst;
    msg->button_io[5] = swst;
	*/
	A27SetReadChecksum(msg);
}

void KeyIO_Init(void){
    memset(&keystate,0,sizeof(struct iostate));
    pthread_create(&hthread, 0, input_thread, 0);
}
