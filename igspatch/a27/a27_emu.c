#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dlfcn.h>

#include "a27.h"
#include "a27_utils.h"

const int fake_pccard_fd = 0x0A271337;
#define PCI_CARD_VERSION 100
#define A27_IO_MAX_CHANNELS 6

static struct A27_Read_Message a27_state = {0};
static unsigned char a27_emu_iotest[68] = {0};
static unsigned char a27_emu_lighttest[68] = {0};

static unsigned short (*A27KeyIO_GetCoinCounter)(void) = NULL;
static void (*A27KeyIO_GetTestBuffer)(unsigned char* io_test_buffer) = NULL;

void load_a27keyio_emu(){
    void* a27keyio_lib = dlopen("./a27_keyio.so",RTLD_NOW);    
    if(!a27keyio_lib){
        printf("[IGSTools::loadlib_A27KeyIO] Error: Could not Open library.\n");
        exit(-1);
    }

    A27KeyIO_GetCoinCounter = dlsym(a27keyio_lib,"A27KeyIO_GetCoinCounter");
    A27KeyIO_GetTestBuffer = dlsym(a27keyio_lib,"A27KeyIO_GetTestBuffer");
    if(!A27KeyIO_GetTestBuffer || !A27KeyIO_GetCoinCounter){
        printf("[IGSTools::loadlib_A27KeyIO] Error: Could not bind to functions.\n");
        exit(-1);
    }
}

void mode_process(const unsigned char* in_data){
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
    
    *(unsigned short*)a27_state.data = resval;
    *(unsigned short*)(a27_state.data+2) = val;

    a27_state.dwBufferSize = 4;
}

static int interval_ctr = 0;
static const int target_interval = 6;
static int time_val = -1;
static int last_time_val = -1;

unsigned char data_6e48[] = {
	0x02, 0x81, 0x00, 0x00, 0x30, 0x01, 0x00, 0x00, // 2 is blue, 0x42 is center 
    0x42, 0x81, 0x00, 0x00, 0x60, 0x00, 0x00, 0x00, // 2 is blue, 0x42 is center 
    0x82, 0x81, 0x00, 0x00, 0x70, 0x00, 0x00, 0x00, // 2 is blue, 0x42 is center 
};

void A27Emu_SongProcess(const unsigned char* in_data, unsigned int in_length){
    unsigned short song_cmd = 0;
    if(!in_length){
        song_cmd = *(unsigned char*)0x83EB861;
    }else{
        song_cmd = *(unsigned short*)in_data;
    }
    

    switch(song_cmd){
        case 3:
            // Song Request Packet - 96 bytes detailing the song id, difficulty, etc.
            *(unsigned short*)a27_state.data = 3;
            a27_state.dwBufferSize = 0xA00;        

            interval_ctr = 0;
            time_val = -1;
            
            //pthread_create(&song_timer_thread, 0, song_timer, 0);
            break;
        case 4:
            *(unsigned short*)a27_state.data = 4;
            a27_state.dwBufferSize = 0xA00;
            break;
        case 5:
        case 6:
            last_time_val = time_val;
            interval_ctr++;
            if(interval_ctr >= target_interval){
                interval_ctr = 0;
                time_val++;
            }

            // Unk Values
           // if(time_val == 915){time_val = -1;}
            *(short*)(a27_state.data+4) = time_val;  
            *(short*)(a27_state.data+6) = time_val;

            *(unsigned short*)a27_state.data = 6;
            // Status
            *(unsigned short*)(a27_state.data+2) = 0;
            /*
            if(time_val == -1){
                time_val++;
            }
             */

            // The next 64 bytes are known as the "SoundIndex" - I've never seen it used.
            memset(a27_state.data+8,0x00,64);

            if(time_val == 10 && last_time_val != time_val){
                //a27_state.data[0x28] = 1;
                a27_state.data[0x28] = 7;
            }   
            
                  
            
            if(time_val % 2){
            //    a27_state.data[0x48] = 0x00;
             //   a27_state.data[0x4F8] = 0x00;
            }else{
               // a27_state.data[0x48] = 0x02;
               // a27_state.data[0x4F8] = 0x02;
            }
            
            memcpy(a27_state.data+0x48,data_6e48,sizeof(data_6e48));

            //a27_state.data[0x48] = (iost.p1_drum_1) ? 0:0x02;
            //memcpy(a27_state.data+0x70,data_6e48,sizeof(data_6e48));
            //memcpy(a27_state.data+0x98,data_6e48,sizeof(data_6e48));
            //memcpy(a27_state.data+0x4D0,data_6e48,sizeof(data_6e48));
            //memcpy(a27_state.data+0x48,data_6e48,sizeof(data_6e48));
            /*
            a27_state.data[0x48] = (iost.p1_drum_1) ? 0x02:0;
            a27_state.data[0x49] = (iost.p1_drum_2) ? 0x40:0x40;
            a27_state.data[0x4A] = (iost.p1_drum_3) ? 0x40:0x40;
            a27_state.data[0x4B] = (iost.p1_drum_4) ? 0:0;
            
 */
            //*(short*)(a27_state.data+0x4C) = time_val;
           // *(short*)(a27_state.data+0x48) = time_val;
            //a27_state.data[0x48] = (unsigned char)(time_val / 10);
            
            //printf("State: %02X\n",time_val );
            
/*
            
            a27_state.data[0x62] = (iost.p2_drum_1) ? 0x02:0;
            a27_state.data[0x62] = (iost.p2_drum_2) ? 0x04:0;
            a27_state.data[0x62] = (iost.p2_drum_3) ? 0x08:0;
            a27_state.data[0x62] = (iost.p2_drum_4) ? 0x10:0;
            a27_state.data[0x62] = (iost.p2_drum_5) ? 0x82:0;
            a27_state.data[0x62] = (iost.p2_drum_6) ? 0x42:0;
            
 */
          // *(short*)(a27_state.data+0x64) = time_val*6;
          // *(short*)(a27_state.data+0x6C) = time_val*2;

           // a27_state.data[0x49] = 0x40;
           // a27_state.data[0x4A] = 0x40;
           // a27_state.data[0x4B] = 0;

           
            //*(short*)(a27_state.data+0x4B) = time_val;
            
             
            // There are two blocks about 0x4B0 in size.
            /*
            a27_state.data[0x4F8] = a27_state.data[0x48];
            a27_state.data[0x4F9] = 0x40;
            a27_state.data[0x4FA] = 0x40;
            a27_state.data[0x4FB] = 0;
          */
            /*
            a27_state.data[0x4FC] = 0x73;
            
            if(time_val == 0x6E){
                memcpy(a27_state.data+0x48,data_6e48,sizeof(data_6e48));
                //memcpy(a27_state.data+0x4F8,data_6e4f8,sizeof(data_6e4f8));
            }
             */

            // 9a8 is combo counter - 9aa is for p2
    
            *(unsigned short*)(a27_state.data+0x9A8) = 69;
            *(unsigned short*)(a27_state.data+0x9AA) = 420;


            // 9ab -> 0x9b0
            
            // Is Player N Playing (if set to 0, the score won't be recorded and the song won't move).
            a27_state.data[0x9B0] = 1;
            a27_state.data[0x9B1] = 1;

            // 9b2-> 9e4 somewhere in here are flags for when you hit the drum - will have to hook these up to the io
            
            //memset(a27_state.data+0x9c0,1,4);

            
            //a27_state.data[0x9B5] = (iost.p1_drum_2) ? 1:0;
            // 0x9b4 blue great, 9b5-8 green great 9b9 red great 1 great, 2 cool, 3  nice, 4 poor, 5 lost, 6 bravo, 8 is the red effect for the byte? 9 is the same, 10 crashes so I guess we get 9 lol
            /*
            a27_state.data[0x9B6] = (iost.p1_drum_3) ? 1:0;
            a27_state.data[0x9B7] = (iost.p1_drum_4) ? 1:0;
            a27_state.data[0x9B8] = (iost.p1_drum_5) ? 1:0;
            a27_state.data[0x9B9] = (iost.p1_drum_6) ? 1:0;
             */
            // 9e4 is player 1 score
            *(unsigned int*)(a27_state.data+0x9E4) = 6969;
            *(unsigned int*)(a27_state.data+0x9E8) = 14;
            // These are score copies... not sure why
            *(unsigned int*)(a27_state.data+0x9EC) = 0;
            *(unsigned int*)(a27_state.data+0x9F0) = 0;


            *(unsigned int*)(a27_state.data+0x9F4) = 0;
            
            // BGA Cues - 4 slots
            *(unsigned short*)(a27_state.data+0x9F8) = 10;
            *(unsigned short*)(a27_state.data+0x9FA) = 10;
            *(unsigned short*)(a27_state.data+0x9FC) = 0;
            *(unsigned short*)(a27_state.data+0x9FE) = 0;
            


            
                /*
            time_ctr++;
            if(time_ctr == time_interval){
                time_ctr=0;
                time_val++;
            }
             */
            a27_state.dwBufferSize = 0xA00;

            //print_hex(a27_state.data,0x4C);
            break;
        
        case 7:
        case 8:
        case 10:
        case 12:
            a27_state.dwBufferSize = 0xA00;
            break;
        case 9:            
            *(unsigned short*)a27_state.data = 9;            
            a27_state.data[0x4C] = 3;
            a27_state.data[0x4D] = 3;
            a27_state.data[0x4E] = 2;
            a27_state.data[0x4F] = 2;
            a27_state.dwBufferSize = 80;
            time_val = -1;
            break;
        default:
            printf("[A27Emu_SongProcess] Error: Unhandled Command: %d\n",song_cmd);
            if(in_length){
                //print_hex(in_data,in_length);
            }
            exit(-1);
    }
}

void A27Emu_Process(const struct A27_Write_Message* msg){

    if(msg->system_mode != a27_state.system_mode){
        printf("Change A27 System Mode to %d\n data: %d\n",msg->system_mode,msg->dwBufferSize );
    }
 
    // Pretty sure we always respond with the requested system mode.
    a27_state.system_mode = msg->system_mode;
    
    switch(msg->system_mode){
        case 0:
        case 6:
        case 7:
        case 8:
        case 9:
        case 10:
        case 12:
        case 16:
        case 17:
        case 18:
        case 19:
        case 21:
        case 22:
        case 23:
        case 24:
        case 26:
        case 27:
            a27_state.dwBufferSize = msg->dwBufferSize;
            break;
        case A27_MODE_KEY_TEST:
            a27_state.dwBufferSize = sizeof(a27_emu_iotest);
            A27KeyIO_GetTestBuffer(a27_state.data);
            break;
        case A27_MODE_LIGHT_TEST:
            //print_hex(msg->data,msg->dwBufferSize);
            // TODO - Light Test Packets
            a27_state.dwBufferSize = sizeof(a27_emu_lighttest);
            memcpy(a27_state.data,a27_emu_lighttest,sizeof(a27_emu_lighttest));
            break;
        case A27_MODE_COUNTER_TEST:
            a27_state.dwBufferSize = 4;
            *(unsigned short*)a27_state.data = 2;
            *(unsigned short*)(a27_state.data+2) = A27KeyIO_GetCoinCounter();
            break;
        case A27_MODE_COIN:
        case A27_MODE_SELECT_MODE:
        case A27_MODE_SELECT_SONG:
        case A27_MODE_RANKING:
            mode_process(msg->data);
            break;  
        case A27_MODE_SONG:
            //print_hex(msg->data,msg->dwBufferSize);
            A27Emu_SongProcess(msg->data,msg->dwBufferSize);
            break;                            
        case A27_MODE_CCD_TEST:
            a27_state.system_mode = A27_MODE_CCD_TEST;
            a27_state.dwBufferSize = 4;
            *(unsigned short*)a27_state.data = 0;
            *(unsigned short*)(a27_state.data+2) = 0;
            break;
        default:
            printf("Unhandled A27 Operation: %d\n", msg->system_mode);
            exit(-1);
        break;
    }
}


void A27Emu_Reset(void){
    printf("[A27Emu::A27_Reset]\n");
    a27_state.dwBufferSize = 0;
    a27_state.system_mode = 0;
    a27_state.coin_inserted = 0;
    a27_state.asic_iserror = 0;
    a27_state.asic_errnum = 5;
    for(int i = 0; i < 6; i++){
        a27_state.button_io[i] = 0;
    }
    a27_state.num_io_channels = A27_IO_MAX_CHANNELS;
    a27_state.protection_value = rand() & 0xFF;
    a27_state.protection_offset = A27DeriveChallenge(a27_state.protection_value);
    a27_state.game_region = REGION_AMERICA;
    a27_state.align_1 = REGION_AMERICA;
    strcpy(a27_state.pch_in_rom_version_name,"S106US");
    strcpy(a27_state.pch_ext_rom_version_name,"E108US");
    a27_state.inet_password_data = 0xFFFF;
    a27_state.a27_has_message = 0;
    a27_state.is_light_io_reset = 0;
    a27_state.pci_card_version = PCI_CARD_VERSION;
    memset(a27_state.a27_message,0,sizeof(a27_state.a27_message));
    memset(a27_state.data,0,sizeof(a27_state.data));
}


// --- Exports ---
int A27Emu_Open(void){
    if(getenv("PM_KEYBOARDIO")){
        load_a27keyio_emu();
    }
    return fake_pccard_fd;
}
int A27Emu_Read(void* buf){
    A27SetChecksum(&a27_state);
    memcpy((unsigned char*)buf,&a27_state,A27_READ_HEADER_SIZE);
    memcpy((unsigned char*)buf+A27_READ_HEADER_SIZE,a27_state.data,a27_state.dwBufferSize);
    return PCCARD_READ_OK; 
}
int A27Emu_Write(const void* buf,size_t count){
    if(count == 0){
        A27Emu_Reset();
    }else{
        A27Emu_Process((const struct A27_Write_Message*)buf);
    }
    return 1;
}
