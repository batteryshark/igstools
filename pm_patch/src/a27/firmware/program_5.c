#include <string.h>
#include "../emulator.h"

// Program 5: Read Data from Trackball(s) in Test Menu [QATest Only]
void A27_Program_5(PA27State a27_state){
    a27_state->msg.header.system_mode = A27_MODE_TRACKBALL;
    a27_state->msg.header.data_size = sizeof(a27_state->trackball);
    
    // We'll just put some example data in here for now.
    a27_state->trackball[0].direction = 2;
    a27_state->trackball[0].vx = 99;
    a27_state->trackball[0].vy = 100;
    a27_state->trackball[0].pulse = 1;

    a27_state->trackball[1].direction = 4;
    a27_state->trackball[1].vx = 14;
    a27_state->trackball[1].vy = 54;
    a27_state->trackball[1].press = 1;

    memcpy(a27_state->msg.data,&a27_state->trackball,a27_state->msg.header.data_size);
}
