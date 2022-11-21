#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>
#include <pthread.h>

#include "pm_keyboardio.h"

#include "a27.h"

static struct A27_Read_Message a27_state = {0};
static struct A27_Write_Message a27_in = {0};
static pthread_t song_timer_thread;

static unsigned char a27_emu_iotest[68] = {0};
static unsigned char a27_emu_lighttest[68] = {0};

// Helpers
void print_hex(unsigned char* data, unsigned int len) {
        unsigned int i;
	for (i = 0; i < len; i++) {
		printf("%02X", data[i]);
	}
	printf("\n");
}


unsigned char derive_a27_challenge_offset(unsigned char inval){
    unsigned char target_value = inval % 10;
    for(int i=0;i<sizeof(a27_protect_table);i++){
        if(a27_protect_table[i] == target_value){
            return i;
        }
    }
    return 0;
}

void a27_set_checksum(struct A27_Read_Message* msg){
    unsigned int cval = (msg->a27_has_message & 0xFF) + \
        (msg->inet_password_data & 0xFF) + \
        msg->is_light_io_reset + \
        (msg->asic_errnum & 0xFF) + \
        msg->asic_iserror + \
        msg->coin_inserted + \
        (msg->dwBufferSize & 0xFF) + \
        (msg->system_mode & 0xFF);
    msg->checksum_1 = (cval & 0xFF);
    msg->checksum_2 = (cval & 0xFF);

}

void A27Emu_ModeProcess(unsigned char* in_data){
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


static int time_val = -1;
static int last_time_val = -1;
static long long time_interval = 100;
static int time_ctr = 0;
unsigned short song_id = 0x0A;
static char in_song = 0;
long long song_ts,n_ts;
static int target_interval = 6;
static int interval_ctr = 0;
long long timeInMilliseconds(void) {
    struct timeval tv;

    gettimeofday(&tv,NULL);
    return (((long long)tv.tv_sec)*1000)+(tv.tv_usec/1000);
}
/*
gettimeofday(&tv_n,NULL);
            if((1000000 * tv_n.tv_sec + tv_n.tv_usec) - (1000000 * tv_start.tv_sec + tv_start.tv_usec) > time_interval){
                time_val++;
                tv_start = tv_n;
            }
 */

static void *song_timer(void *arg){
time_val = -1;

while(!in_song){}
long long tv_start = timeInMilliseconds();
while(in_song){
            long long tv_n = timeInMilliseconds();
            if((tv_n - tv_start) >= time_interval){
               // time_val++;
                tv_start = tv_n;
            }
            
            
}
}

unsigned char data_6e48[40] = {
	0x42, 0x40, 0x00, 0x00, 0x73, 0x00, 0x00, 0x00, 0x02, 0x40, 0x40, 0x00, 0x73, 0x00, 0x00, 0x00, 
	0x42, 0x40, 0x00, 0x00, 0x53, 0x01, 0x00, 0x00, 0x42, 0x40, 0x00, 0x00, 0x3A, 0x00, 0x00, 0x00, 
	0x42, 0x40, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00
};


unsigned char data_6e4f8[40] = {
	0x42, 0x40, 0x00, 0x00, 0x74, 0x00, 0x00, 0x00, 0x02, 0x40, 0x40, 0x00, 0x74, 0x00, 0x00, 0x00, 
	0x42, 0x40, 0x00, 0x00, 0x54, 0x01, 0x00, 0x00, 0x42, 0x40, 0x00, 0x00, 0x3B, 0x00, 0x00, 0x00, 
	0x42, 0x40, 0x00, 0x00, 0x07, 0x00, 0x00, 0x00
};

void A27Emu_SongProcess(unsigned char* in_data, unsigned int in_length){
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
            
            time_ctr = 0;

            interval_ctr = 0;
            time_val = -1;
            
            pthread_create(&song_timer_thread, 0, song_timer, 0);
            break;
        case 4:
            *(unsigned short*)a27_state.data = 4;
            a27_state.dwBufferSize = 0xA00;
            break;
        case 5:
        case 6:
            if(!in_song){
                in_song = 1;
                
            }


            // Unk Values
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
                a27_state.data[0x28] = 7;
            }   
            
            
            if(time_val % 2){
                a27_state.data[0x48] = 0x00;
            }else{
                a27_state.data[0x48] = 0x02;
            }
            


            a27_state.data[0x49] = 0x40;
            a27_state.data[0x4A] = 0x40;
            a27_state.data[0x4B] = 0;

           
            *(short*)(a27_state.data+0x4B) = time_val;
            
             
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

            a27_state.data[0x9A8] = 0x0C;
            a27_state.data[0x9AA] = 0x0C;
            a27_state.data[0x9E4] = 0x09;
            a27_state.data[0x9E5] = 0x01;
            a27_state.data[0x9E8] = 0x09;
            a27_state.data[0x9E9] = 0x01;
            a27_state.data[0x9EC] = 0x09;
            a27_state.data[0x9ED] = 0x01;  
            a27_state.data[0x9F0] = 0x09;
            a27_state.data[0x9F1] = 0x01;                                    
            a27_state.data[0x9F8] = song_id;
            a27_state.data[0x9FA] = song_id;
            a27_state.data[0x9B0] = 2;
            a27_state.data[0x9B1] = 1;

            
                /*
            time_ctr++;
            if(time_ctr == time_interval){
                time_ctr=0;
                time_val++;
            }
             */
            a27_state.dwBufferSize = 0xA00;
            last_time_val = time_val;
            interval_ctr++;
            if(interval_ctr >= target_interval){
                interval_ctr = 0;
                time_val++;
            }
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
            in_song = 0;
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
                print_hex(in_data,in_length);
            }
            exit(-1);
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
    a27_state.num_io_channels = 6;
    a27_state.protection_value = rand() & 0xFF;
    a27_state.protection_offset = derive_a27_challenge_offset(a27_state.protection_value);
    a27_state.game_region = REGION_AMERICA;
    a27_state.align_1 = REGION_AMERICA;
    strcpy(a27_state.pch_in_rom_version_name,"S106US");
    strcpy(a27_state.pch_ext_rom_version_name,"E108US");
    a27_state.inet_password_data = 0xFFFF;
    a27_state.a27_has_message = 0;
    a27_state.is_light_io_reset = 0;
    a27_state.pci_card_version = 0x64;
    memset(a27_state.a27_message,0,sizeof(a27_state.a27_message));
    memset(a27_state.data,0,sizeof(a27_state.data));
}


ssize_t A27Emu_Read(unsigned char* buf){
    a27_set_checksum(&a27_state);
    memcpy((unsigned char*)buf,&a27_state,A27_READ_HEADER_SIZE);
    memcpy((unsigned char*)buf+A27_READ_HEADER_SIZE,a27_state.data,a27_state.dwBufferSize);
    return PCCARD_READ_OK;    
}

void A27InjectKeyboardIO(unsigned char* buf){
    
    
    struct A27_Read_Message* msg = (struct A27_Read_Message*)buf;
    msg->num_io_channels = 6;
    unsigned int io_state = get_emulated_drumio_state(1);
    msg->button_io[2] = io_state;
    msg->button_io[3] = io_state;
    msg->button_io[4] = io_state;
    msg->button_io[5] = io_state;

    msg->coin_inserted = get_emulated_coin_state(1);
    
    // Because we've modified the packet, we now have to reset the checksum.
    a27_set_checksum(msg);
}

void A27PrintRead(ssize_t res,unsigned char* buf){
    struct A27_Read_Message* msg = (struct A27_Read_Message*)buf;

    printf("[A27Log] Read Res: %d Data Amt: %d\n",res,msg->dwBufferSize);
    // TODO - Figure out when we're writing another packet and how we filter.
    if(msg->dwBufferSize){
        print_hex(msg->data,msg->dwBufferSize);
    }
}
void A27PrintWrite(ssize_t res, const void* buf, size_t count){
    struct A27_Write_Message* msg = (struct A27_Write_Message*)buf;
    printf("[A27Log] Write Data Res: %d Amt: %d\n",res, msg->dwBufferSize);
    // TODO - Figure out when we're writing another packet and how we filter.
    if(msg->dwBufferSize){
        print_hex(msg->data,msg->dwBufferSize);
    }  
}

void A27Emu_Process(const void* buf,size_t count){
    struct A27_Write_Message* msg = (struct A27_Write_Message*)buf;
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
            get_io_test_buffer(a27_state.data);
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
            *(unsigned short*)(a27_state.data+2) = get_emulated_coin_counter();
            break;
        case A27_MODE_COIN:
        case A27_MODE_SELECT_MODE:
        case A27_MODE_SELECT_SONG:
        case A27_MODE_RANKING:
            A27Emu_ModeProcess(msg->data);
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
    

    // TODO - All the Things!
    return;
}