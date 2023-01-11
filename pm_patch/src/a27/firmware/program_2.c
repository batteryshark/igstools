#include <string.h>
#include "../emulator.h"
#include "../../io/keyio.h"


// Program 2: Read Key Input Test Page
// This program executes on the input test screen to show states of the various switches.
void A27_Program_2(PA27State a27_state){
    a27_state->msg.header.system_mode = A27_MODE_KEY_TEST;
    unsigned char io_test_buffer[68] = {0x00};

    // This is the status code.
    *(unsigned short*)io_test_buffer = 0;
    
    unsigned int sw = a27_state->msg.header.button_io[0];
    
    io_test_buffer[4]  = IO_ISSET(sw,INP_P1_DRUM_L);
    io_test_buffer[5]  = IO_ISSET(sw,INP_P1_DRUM_R); 
    io_test_buffer[6]  = IO_ISSET(sw,INP_P1_RIM_R); 
    io_test_buffer[9]  = IO_ISSET(sw,INP_P1_BLUE);
    io_test_buffer[10] = IO_ISSET(sw,INP_P1_RED);
    io_test_buffer[11] = IO_ISSET(sw,INP_P1_RIM_L);

    io_test_buffer[12] = IO_ISSET(sw,INP_P2_BLUE); 
    io_test_buffer[13] = IO_ISSET(sw,INP_P2_RED); 
    io_test_buffer[14] = IO_ISSET(sw,INP_P2_RIM_L); 
    io_test_buffer[15] = IO_ISSET(sw,INP_P2_DRUM_L); 
    io_test_buffer[16] = IO_ISSET(sw,INP_P2_DRUM_R); 
    io_test_buffer[17] = IO_ISSET(sw,INP_P2_RIM_R); 

    io_test_buffer[33] = a27_state->msg.header.coin_inserted;
    io_test_buffer[34] = IO_ISSET(sw,INP_SW_SERVICE);
    io_test_buffer[35] = IO_ISSET(sw,INP_SW_TEST);
    
    // Copies to deal with counts. Likely "Last State"
    io_test_buffer[36] = io_test_buffer[4];
    io_test_buffer[37] = io_test_buffer[5];
    io_test_buffer[38] = io_test_buffer[6];
    io_test_buffer[41] = io_test_buffer[9];
    io_test_buffer[42] = io_test_buffer[10];
    io_test_buffer[43] = io_test_buffer[11];

    io_test_buffer[44] = io_test_buffer[12];
    io_test_buffer[45] = io_test_buffer[13];
    io_test_buffer[46] = io_test_buffer[14];
    io_test_buffer[47] = io_test_buffer[15];
    io_test_buffer[48] = io_test_buffer[16];
    io_test_buffer[49] = io_test_buffer[17];

    io_test_buffer[65] = io_test_buffer[33];
    io_test_buffer[66] = io_test_buffer[34];
    io_test_buffer[67] = io_test_buffer[35];
    
    a27_state->msg.header.data_size = sizeof(io_test_buffer);
    memcpy(a27_state->msg.data,io_test_buffer,a27_state->msg.header.data_size);

}
