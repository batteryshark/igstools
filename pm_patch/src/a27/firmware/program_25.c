#include <string.h>

#include "../emulator.h"

// Program 25: CCD (Webcam) Test Page
// The drivers are super finnicky. This probably does more than return an "OK".
void A27_Program_25(PA27State a27_state){
    a27_state->msg.header.system_mode = A27_MODE_CCD_TEST;
    a27_state->msg.header.data_size = 4;
    memset(a27_state->msg.data,0,4);
}
