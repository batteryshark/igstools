#include <stdio.h>
#include <stdlib.h>

// X11 includes
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <X11/Xatom.h>
#include <X11/keysym.h>

#include "keyio.h"



void x11_input_loop(PKeyIOState key_state, PKeyIOState last_key_state){
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
        XNextEvent(display, &event);
        if (event.type == KeyPress || event.type == KeyRelease){
            ksym = XLookupKeysym(&event.xkey, 0);         
            if(ksym == XK_Escape){
                printf("[A27_KeyIO] User Exited.\n");
                return;
            }
            memcpy(last_key_state,key_state,sizeof(KeyIOState));
            switch(ksym){            
                // System Switches
                case XK_1:
                    key_state->system[KEYIO_STEST] = (event.type == KeyPress) ? 1 : 0;
                    break;
                case XK_2:
                    key_state->system[KEYIO_SSVC] = (event.type == KeyPress) ? 1 : 0;
                    break;
                case XK_3:
                    key_state->system[KEYIO_DEV_1] = (event.type == KeyPress) ? 1:0;
                    break;
                case XK_4:
                    key_state->system[KEYIO_DEV_2] = (event.type == KeyPress) ? 1:0;
                    break;
                case XK_5:
                    key_state->system[KEYIO_COIN] = (event.type == KeyPress) ? 1 : 0;
                    break;                  
                // Player 1 
                case XK_z:
                    key_state->p1[KEYIO_PRIM_L] = (event.type == KeyPress) ? 1 : 0;
                    break;
                case XK_x:
                    key_state->p1[KEYIO_PDRUM_L] = (event.type == KeyPress) ? 1 : 0;
                    break;
                case XK_c:
                    key_state->p1[KEYIO_PDRUM_R] = (event.type == KeyPress) ? 1 : 0;
                    break;
                case XK_v:
                    key_state->p1[KEYIO_PRIM_R] = (event.type == KeyPress) ? 1 : 0;
                    break;
                case XK_s:
                    key_state->p1[KEYIO_PBLUE] = (event.type == KeyPress) ? 1 : 0;
                    break;
                case XK_f:
                    key_state->p1[KEYIO_PRED] = (event.type == KeyPress) ? 1 : 0;
                    break;
                // Player 2 
                case XK_b:
                    key_state->p2[KEYIO_PRIM_L] = (event.type == KeyPress) ? 1 : 0;
                    break;
                case XK_n:
                    key_state->p2[KEYIO_PDRUM_L] = (event.type == KeyPress) ? 1 : 0;
                    break;
                case XK_m:
                    key_state->p2[KEYIO_PDRUM_R] = (event.type == KeyPress) ? 1 : 0;
                    break;
                case XK_comma:
                    key_state->p2[KEYIO_PRIM_R] = (event.type == KeyPress) ? 1 : 0;
                    break;
                case XK_h:
                    key_state->p2[KEYIO_PBLUE] = (event.type == KeyPress) ? 1 : 0;
                    break;
                case XK_k:
                    key_state->p2[KEYIO_PRED] = (event.type == KeyPress) ? 1 : 0;
                    break;                                                                                               
                default:
                break;
            }            
        }
    }
}
