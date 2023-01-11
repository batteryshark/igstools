#include "../emulator.h"


// Program 4: Read Coin Counter State in Test Menu
void A27_Program_4(PA27State a27_state){
    a27_state->msg.header.system_mode = A27_MODE_COUNTER_TEST;
    a27_state->msg.header.data_size = 4;
    *(unsigned short*)a27_state->msg.data = 2;
    *(unsigned short*)(a27_state->msg.data+2) = a27_state->coin_counter;
}
