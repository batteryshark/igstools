#include <string.h>
#include "../emulator.h"

// Program 1: ReadWrite Test Page 
// It essentially increments a counter and prints out some arbitrary text.
// This isn't really used in the game (only in devtest).
void A27_Program_1(PA27State a27_state){
    a27_state->msg.header.system_mode = A27_MODE_READWRITE_TEST;
    if(a27_state->test_text[1].counter == 0xFFFF){
        a27_state->test_text[1].counter = 0;
    }
    a27_state->test_text[1].counter++;
    memcpy(a27_state->msg.data,&a27_state->test_text,sizeof(a27_state->test_text)); 
    a27_state->msg.header.data_size = sizeof(a27_state->test_text);
}
