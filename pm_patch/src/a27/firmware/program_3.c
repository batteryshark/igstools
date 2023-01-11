#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../emulator.h"
#include "../../io/keyio.h"

enum A27_LightTest{
    LIGHT_2P_3 = 4,
    LIGHT_YB79,
    LIGHT_YB810,
    LIGHT_2P_1,
    LIGHT_2P_2,
    LIGHT_1P_1 = 12,
    LIGHT_1P_2,
    LIGHT_1P_3,
    LIGHT_YA1,
    LIGHT_YA2,
    LIGHT_YA3,
    LIGHT_YA4,
};

const unsigned char light_test_1_offsets[] = {LIGHT_YA1,LIGHT_YA2,LIGHT_YA3,LIGHT_YA4,LIGHT_YB79,LIGHT_YB810};
const unsigned char light_test_2_offsets[] = {LIGHT_1P_1,LIGHT_1P_2,LIGHT_1P_3,LIGHT_2P_1,LIGHT_2P_2,LIGHT_2P_3};
const unsigned char strobe_max = 60;

typedef struct _LIGHT_TEST_STATE{
    unsigned char mode;
    unsigned char offset;
    unsigned char strobe_count;
}LightTestState,*PLightTestState;

static LightTestState tstate;

typedef struct _A27_PROGRAM_3_Request{
    unsigned short status;
    unsigned short subcmd;
}A27Progam3Request,*PA27Program3Request;

// Program 3: Write Light Configuration Feedback from Light IO Test Page
// Note: Light Test 2 (the indicators for drum lights) aren't used in the automated test rotation. I think you have to hit the drum to activate those.
void A27_Program_3(PA27WriteMessage req, PA27ReadMessage res){
    int light_test_len_offset = (tstate.mode) ? sizeof(light_test_2_offsets) : sizeof(light_test_1_offsets);
    res->header.system_mode = A27_MODE_LIGHT_TEST;
    // Not sure if this will ever be needed.
    if(req->header.data_size < 4){return;}
    PA27Program3Request p3r = (PA27Program3Request)req->data;
    const unsigned char* selected_offset = (tstate.mode) ? light_test_2_offsets : light_test_1_offsets;
    if(p3r->status == 1 && p3r->subcmd == 0x41 && req->header.data_size == 68){
        tstate.strobe_count++;
        if(tstate.strobe_count == strobe_max){
            tstate.strobe_count = 0;
            tstate.offset = (tstate.offset + 1) % light_test_len_offset;            
        }
        memset(res->data,0,68);
        res->header.data_size = 68;
        *(unsigned short*)res->data = 1;
        *(unsigned short*)(res->data+2) = 4;                      
        res->data[selected_offset[tstate.offset]] = 1;  

        unsigned int swst = res->header.button_io[0];
        res->data[LIGHT_1P_1] = IO_ISSET(swst,INP_P1_BLUE) ? 1 : 0;
        res->data[LIGHT_1P_2] = IO_ISSET(swst,INP_P1_RED) ? 1 : 0;
        res->data[LIGHT_1P_3] = IO_ISSET(swst,INP_P1_DRUM_R) ? 1 : 0;
        res->data[LIGHT_2P_1] = IO_ISSET(swst,INP_P2_BLUE) ? 1 : 0;
        res->data[LIGHT_2P_2] = IO_ISSET(swst,INP_P2_RED) ? 1 : 0;
        res->data[LIGHT_2P_3] = IO_ISSET(swst,INP_P2_DRUM_R) ? 1 : 0;
        return;            
    }
        switch(p3r->subcmd){
        case 0:
        case 2:
        case 0x41:
            tstate.mode = 0;
            tstate.offset = 0;
            res->header.data_size = 4;
            *(unsigned short*)res->data = 0;
            *(unsigned short*)(res->data+2) = 4;  
            break;
        
        default:
            printf("Unhandled Light SubCmd: %d\n", p3r->subcmd);
            exit(-1);
            break;
    }
}
