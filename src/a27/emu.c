// IGS PCCARD ASIC Driver for PercussionMaster
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dlfcn.h>

#include "../keyio.h"
#include "../utils.h"


#include "a27.h"
#include "song.h"
#include "emu.h"


// Current Message Body
static struct A27_Read_Message a27_msg = {0};

// Internal State Tracker
static struct _a27_state{
    unsigned short coin_counter;
    TEST_READWRITE_DATA test_text[21];
    TRACKBALL_DATA trackball[2];
}a27_state;

// --- A27 PROGRAMS ---

// ReadWrite Test Page
void Program_1_ReadWrite(void){
    
    if(a27_state.test_text[1].counter == 0xFFFF){
        a27_state.test_text[1].counter = 0;
    }
    a27_state.test_text[1].counter++;
    
    memcpy(a27_msg.data,&a27_state.test_text,sizeof(a27_state.test_text));    
}

// Read Key Input Test Page
void Program_2_KeyTest(void){
    unsigned char io_test_buffer[68] = {0x00};

        // This is the status code.
    *(unsigned short*)io_test_buffer = 0;
    unsigned int sw = a27_msg.button_io[0];
    
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

    io_test_buffer[33] = a27_msg.coin_inserted;
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
    
    a27_msg.dwBufferSize = sizeof(io_test_buffer);
    memcpy(a27_msg.data,io_test_buffer,a27_msg.dwBufferSize);

}

// Write Light Configuration Feedback from Light IO Test Page
// Note: Light Test 2 (the indicators for drum lights) aren't used in the automated test rotation. I think you have to hit the drum to activate those.
unsigned char light_test_1_offsets[] = {LIGHT_YA1,LIGHT_YA2,LIGHT_YA3,LIGHT_YA4,LIGHT_YB79,LIGHT_YB810};
unsigned char light_test_2_offsets[] = {LIGHT_1P_1,LIGHT_1P_2,LIGHT_1P_3,LIGHT_2P_1,LIGHT_2P_2,LIGHT_2P_3};
unsigned char light_test_mode = 0;
unsigned char light_test_offset = 0;
unsigned char light_test_strobe_count = 0;
unsigned char light_test_strobe_max = 60;
void Program_3_LightTest(const unsigned char* wdata,unsigned int wsize){
    // Not sure if this will ever be needed...
    if(wsize < 4){return;}
    unsigned short status = *(unsigned short*)wdata;
    unsigned short subcmd = *(unsigned short*)(wdata+2);
    int light_test_len_offset = (light_test_mode) ? sizeof(light_test_2_offsets) : sizeof(light_test_1_offsets);
    unsigned char* selected_offset = (light_test_mode) ? light_test_2_offsets : light_test_1_offsets;
    if(status == 1 && subcmd == 0x41 && wsize == 68){
   
        light_test_strobe_count++;
        if(light_test_strobe_count == light_test_strobe_max){
            light_test_strobe_count = 0;
            light_test_offset++;
            
            if(light_test_offset == light_test_len_offset){
                light_test_offset = 0;
            }
        }
        memset(a27_msg.data,0x00,68);

        a27_msg.dwBufferSize = 68;
        *(unsigned short*)a27_msg.data = 1;
        *(unsigned short*)(a27_msg.data+2) = 4;                      
        a27_msg.data[selected_offset[light_test_offset]] = 1;  

        unsigned int swst = a27_msg.button_io[0];
        a27_msg.data[LIGHT_1P_1] = IO_ISSET(swst,INP_P1_BLUE) ? 1 : 0;
        a27_msg.data[LIGHT_1P_2] = IO_ISSET(swst,INP_P1_RED) ? 1 : 0;
        a27_msg.data[LIGHT_1P_3] = IO_ISSET(swst,INP_P1_DRUM_R) ? 1 : 0;
        a27_msg.data[LIGHT_2P_1] = IO_ISSET(swst,INP_P2_BLUE) ? 1 : 0;
        a27_msg.data[LIGHT_2P_2] = IO_ISSET(swst,INP_P2_RED) ? 1 : 0;
        a27_msg.data[LIGHT_2P_3] = IO_ISSET(swst,INP_P2_DRUM_R) ? 1 : 0;

        return;    
    }
    switch(subcmd){
        case 0:
        case 2:
        case 0x41:
            light_test_mode = 0;
            light_test_offset = 0;
            a27_msg.dwBufferSize = 4;
            *(unsigned short*)a27_msg.data = 0;
            *(unsigned short*)(a27_msg.data+2) = 4;
            break;
        
        default:
            printf("Unhandled Light SubCmd: %d\n", subcmd);
            exit(-1);
            break;
    }
}

// Read Coin Counter State in Test Menu
void Program_4_CounterTest(void){
    a27_msg.dwBufferSize = 4;
    *(unsigned short*)a27_msg.data = 2;
    *(unsigned short*)(a27_msg.data+2) = a27_state.coin_counter;    
}

// Read Data from Trackball(s) in Test Menu
void Program_5_TrackBallTest(void){
    a27_msg.dwBufferSize = sizeof(a27_state.trackball);
    // We'll just put some example data in here for now.
    a27_state.trackball[0].direction = 2;
    a27_state.trackball[0].vx = 99;
    a27_state.trackball[0].vy = 100;
    a27_state.trackball[0].pulse = 1;

    a27_state.trackball[1].direction = 4;
    a27_state.trackball[1].vx = 14;
    a27_state.trackball[1].vy = 54;
    a27_state.trackball[1].press = 1;

    memcpy(a27_msg.data,&a27_state.trackball,a27_msg.dwBufferSize);
}

// Various Screen State Mode Changes
void Program_11_13_14_20_ScreenSelect(const unsigned char* in_data){
    unsigned short wcmdwrite = *(unsigned short*)in_data;
    unsigned short val =  *(unsigned short*)(in_data+2);
    unsigned short resval = 0;
    switch(wcmdwrite){
        case 0:
            resval = 1;
            break;
        case 1:
            resval = 2;
            break;
        case 2:
        case 3:
            resval = 3;
            break;
        default:
            break;
    }
    
    *(unsigned short*)a27_msg.data = resval;
    *(unsigned short*)(a27_msg.data+2) = val;

    a27_msg.dwBufferSize = 4;    
}

// Song State Processing
static unsigned short song_cmd = 0;
void Program_15_Song(const unsigned char* in_data, unsigned int in_length){
    if(in_length){
        song_cmd = *(unsigned short*) in_data;
    }
    switch(song_cmd){
        case A27_SONGMODE_PLAYBACK_HEADER:
            Song_UploadPlaybackHeader(in_data,&a27_msg);
            break;
        case A27_SONGMODE_PLAYBACK_BODY:
            Song_UploadPlaybackBody(in_data,&a27_msg);
            break;
        case A27_SONGMODE_MAINGAME_SETTING:
            Song_MainGameSetting(in_data,&a27_msg);
            break;
        case A27_SONGMODE_MAINGAME_WAITSTART:
            Song_MainGameWaitStart(in_data,&a27_msg);
            break;
        case A27_SONGMODE_MAINGAME_START:
            Song_MainGameStart(in_data,&a27_msg);
            break;
        case A27_SONGMODE_MAINGAME_PROCESS:
            Song_MainGameProcess(in_data,&a27_msg);
            break;
        case A27_SONGMODE_RESULT:
            Song_ResultProcess(in_data,&a27_msg);
            break;
        default:
            printf("[A27Emu::SongProcess] Error: Unhandled Subcommand: %d\n",song_cmd);
            if(in_length){PrintHex((unsigned char*)in_data,in_length);}            
            Shutdown();
    }
}

// Read Error State for CCD (Webcam) Test Page.
void Program_25_CCDTest(void){
    a27_msg.dwBufferSize = 4;
    *(unsigned short*)a27_msg.data = 0;
    *(unsigned short*)(a27_msg.data+2) = 0;
}

// --- HELPERS ---
void A27Emu_Process(const struct A27_Write_Message* msg){
    // We should update the coin state if there is an update at this point.
    a27_msg.coin_inserted = KeyIO_GetCoinState();
    if(a27_msg.coin_inserted){
    if(a27_state.coin_counter == 0xFFFF){a27_state.coin_counter = 0;}
        a27_state.coin_counter++;
    }
    // We can update the switch state here, too.
    a27_msg.num_io_channels = 1;    
    a27_msg.button_io[0] = msg->key_input;
    
    // And the system mode + Buffer Size
    a27_msg.system_mode = msg->system_mode;
    a27_msg.dwBufferSize = msg->dwBufferSize;
    
    // The rest depends on the program being executed...
    switch(msg->system_mode){
        case A27_MODE_READWRITE_TEST:            
            Program_1_ReadWrite();
            break;
        case A27_MODE_KEY_TEST:
            Program_2_KeyTest();
            break;
        case A27_MODE_LIGHT_TEST:
            Program_3_LightTest(msg->data,msg->dwBufferSize);
            break;    
        case A27_MODE_COUNTER_TEST:
            Program_4_CounterTest();
            break;  
        case A27_MODE_TRACKBALL:
            Program_5_TrackBallTest();
            break;
        case A27_MODE_COIN:
        case A27_MODE_SELECT_MODE:
        case A27_MODE_SELECT_SONG:
        case A27_MODE_RANKING:
            Program_11_13_14_20_ScreenSelect(msg->data);
            break;  
        case A27_MODE_SONG:
            Program_15_Song(msg->data,msg->dwBufferSize);
            break;
        case A27_MODE_CCD_TEST:
            Program_25_CCDTest();
            break;
        default:// Anything unhandled we don't care about.
            a27_msg.dwBufferSize = msg->dwBufferSize;
            break;
    }
}

// Exports
void A27Emu_Reset(void){
    memset(&a27_state,0,sizeof(struct _a27_state));
    printf("[A27Emu::A27_Reset]\n");
    a27_msg.dwBufferSize = 0;
    a27_msg.system_mode = 0;
    a27_msg.coin_inserted = 0;
    a27_msg.asic_iserror = 0;
    a27_msg.asic_errnum = 5;
    for(int i = 0; i < 6; i++){
        a27_msg.button_io[i] = 0;
    }
    // Technically, the initial reads from the card start the number of IO channels at 6,
    // But once the game starts responding to actual packets, this gets set to 1 or sometimes 3?
    a27_msg.num_io_channels = 1;
    a27_msg.protection_value = rand() & 0xFF;
    a27_msg.protection_offset = A27DeriveChallenge(a27_msg.protection_value);
    a27_msg.game_region = REGION_AMERICA;
    a27_msg.align_1 = REGION_AMERICA;
    strcpy(a27_msg.pch_in_rom_version_name,IN_ROM_NAME);
    strcpy(a27_msg.pch_ext_rom_version_name,EXT_ROM_NAME);
    a27_msg.inet_password_data = 0xFFFF;
    a27_msg.a27_has_message = 0;
    a27_msg.is_light_io_reset = 0;
    a27_msg.pci_card_version = PCI_CARD_VERSION;
    memset(a27_msg.a27_message,0,sizeof(a27_msg.a27_message));
    memset(a27_msg.data,0,sizeof(a27_msg.data));

    // Reset the Test Read/Write Buffers
    a27_state.test_text[0].enabled_flag = 0;
    a27_state.test_text[0].counter = 0;
    strcpy(a27_state.test_text[0].text,"Test1");
    a27_state.test_text[1].enabled_flag = 1;
    a27_state.test_text[1].counter = 0;
    strcpy(a27_state.test_text[1].text,"Test2");

    // Reset Trackball State.
    memset(&a27_state.trackball,0x00,sizeof(TRACKBALL_DATA)*2);
}

void A27Emu_Write(const void* buf){
    A27Emu_Process((const struct A27_Write_Message*)buf);
}

void A27Emu_Read(void* buf){
    A27SetReadChecksum(&a27_msg);    
    memcpy((unsigned char*)buf,&a27_msg,A27_READ_HEADER_SIZE);
    memcpy((unsigned char*)buf+A27_READ_HEADER_SIZE,a27_msg.data,a27_msg.dwBufferSize);
}
