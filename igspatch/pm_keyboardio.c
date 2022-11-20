#include <stdio.h>
#include <linux/input.h>
#include <pthread.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

static unsigned int fuzzinput = 0;
#define IO_READ_DEBOUNCE 4
#define IO_COIN_DEBOUNCE 10
static int io_coin_ctr = 0;
static unsigned int io_ctr = 0;

static unsigned short coin_ctr = 0;
unsigned short get_emulated_coin_counter(void){
    return coin_ctr;
}

unsigned int get_fuzz_input(void){
    return fuzzinput;
}
static struct iostate {
    unsigned char test_switch;
    unsigned char service_switch;
    unsigned char last_coin;
    unsigned char coin;
    unsigned char p1_drum_1;
    unsigned char p1_drum_2;
    unsigned char p1_drum_3;
    unsigned char p1_drum_4;
    unsigned char p1_drum_5;
    unsigned char p1_drum_6;                    
    unsigned char p2_drum_1;
    unsigned char p2_drum_2;    
    unsigned char p2_drum_3;
    unsigned char p2_drum_4;
    unsigned char p2_drum_5;
    unsigned char p2_drum_6;

}iost;

static struct input_event ev;
static pthread_t hthread;

unsigned int get_emulated_drumio_state(unsigned int respect_debounce){
    if(respect_debounce){
        if(io_ctr){
            --io_ctr;
            return 0;
        }
        io_ctr = IO_READ_DEBOUNCE;
    }

    unsigned int niostate = 0;
    
    // Player 1
    (iost.p1_drum_1) ? (niostate |= (1 << 1)) : (niostate &= ~(1 << 1));
    (iost.p1_drum_2) ? (niostate |= (1 << 2)) : (niostate &= ~(1 << 2));
    (iost.p1_drum_3) ? (niostate |= (1 << 7)) : (niostate &= ~(1 << 7));
    (iost.p1_drum_4) ? (niostate |= (1 << 0)) : (niostate &= ~(1 << 0));
    (iost.p1_drum_5) ? (niostate |= (1 << 5)) : (niostate &= ~(1 << 5));
    (iost.p1_drum_6) ? (niostate |= (1 << 6)) : (niostate &= ~(1 << 6));
    
    
    // Player 2
    (iost.p2_drum_1) ? (niostate |= (1 << 8)) : (niostate &= ~(1 << 8));
    (iost.p2_drum_2) ? (niostate |= (1 << 9)) : (niostate &= ~(1 << 9));
    (iost.p2_drum_3) ? (niostate |= (1 << 10)) : (niostate &= ~(1 << 10));
    (iost.p2_drum_4) ? (niostate |= (1 << 11)) : (niostate &= ~(1 << 11));
    (iost.p2_drum_5) ? (niostate |= (1 << 12)) : (niostate &= ~(1 << 12));
    (iost.p2_drum_6) ? (niostate |= (1 << 13)) : (niostate &= ~(1 << 13));

    // Switches
    (iost.service_switch) ? (niostate |= (1 << 30)) : (niostate &= ~(1 << 30));    
    (iost.test_switch) ? (niostate |= (1 << 31)) : (niostate &= ~(1 << 31));
    return niostate;
}



unsigned char get_emulated_coin_state(unsigned int respect_debounce){
    if(respect_debounce){
    if(io_coin_ctr){
        --io_coin_ctr;
        return 0;
    }
    io_coin_ctr = IO_COIN_DEBOUNCE;
    }
    if(iost.coin){
        ++coin_ctr;
        if(coin_ctr == 0xFFFF){coin_ctr = 0;}
    }
    return iost.coin;
}

void get_io_test_buffer(unsigned char* io_test_buffer){
    unsigned int io_state = get_emulated_drumio_state(0);
    unsigned int coin_state = get_emulated_coin_state(0);

    // This is the status code.
    *(unsigned short*)io_test_buffer = 0;

    io_test_buffer[9] = ((io_state >> 1) & 0x01); 
    io_test_buffer[10] = ((io_state >> 2) & 0x01);
    io_test_buffer[11] = ((io_state >> 7) & 0x01);   
    io_test_buffer[4] = ((io_state >> 0) & 0x01);
    io_test_buffer[5] = ((io_state >> 5) & 0x01); 
    io_test_buffer[6] = ((io_state >> 6) & 0x01); 

    io_test_buffer[12] = ((io_state >> 8) & 0x01); 
    io_test_buffer[13] = ((io_state >> 9) & 0x01); 
    io_test_buffer[14] = ((io_state >> 10) & 0x01); 
    io_test_buffer[15] = ((io_state >> 11) & 0x01); 
    io_test_buffer[16] = ((io_state >> 12) & 0x01); 
    io_test_buffer[17] = ((io_state >> 13) & 0x01); 

    io_test_buffer[33] = (iost.coin) ? 1 : 0;
    io_test_buffer[34] = (iost.service_switch) ? 1 : 0;
    io_test_buffer[35] = (iost.test_switch) ? 1 : 0;
    
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

    


}


const char* keyboard_evpaths[] = {
    "/dev/input/keyboard",
    "/dev/input/by-path/platform-i8042-serio-0-event-kbd",
    "/run/kbdhook"
};

static void *input_thread(void *arg){
    int selected_input;
    int num_input_paths = sizeof(keyboard_evpaths)/ sizeof(const char*);
    int fd;
    int res;

    for(selected_input = 0; selected_input < num_input_paths; selected_input++){
        fd = open(keyboard_evpaths[selected_input],0);
        if(fd != -1){
            printf("[IGSTools::KeyIO] Opened Event at: %s\n",keyboard_evpaths[selected_input]);
            break;
        }
    }

    if (fd == -1) {
        printf("[IGSTools::KeyIO] Error Opening input. Possibly not running as elevated or not added to 'input' group.\n");
        exit(-1);
    }
 
    while(1){
        res = read(fd, &ev, sizeof(ev));
        if (ev.type == EV_KEY) {
            if(ev.code == KEY_ESC){ 
                printf("[IGSTools::KeyIO] User Exited.\n");                
                exit(0);
            }
            switch(ev.code){
                // System Controls
                case KEY_1:
                    iost.test_switch = (ev.value) ? 1:0;
                    break;
                case KEY_2:
                    iost.service_switch = (ev.value) ? 1:0;
                    break;                    
                case KEY_5:
                    iost.coin = (ev.value) ? 1:0;
                    break;
                // Player 1 Controls
                case KEY_Z: // 0
                    iost.p1_drum_1 = (ev.value) ? 1:0;
                    break; 
                case KEY_X: // 1
                    iost.p1_drum_2 = (ev.value) ? 1:0;
                    break;
                case KEY_C: // 2
                    iost.p1_drum_3 = (ev.value) ? 1:0;
                    break;
                case KEY_V: // 5
                    iost.p1_drum_4 = (ev.value) ? 1:0;
                    break;
                case KEY_B: // 6
                    iost.p1_drum_5 = (ev.value) ? 1:0;
                    break;                    
                case KEY_N: // 7
                    iost.p1_drum_6 = (ev.value) ? 1:0;
                    break;
                // Player 2 Controls
                case KEY_A:
                    iost.p2_drum_1 = (ev.value) ? 1:0; 
                    break;
                case KEY_S:
                    iost.p2_drum_2 = (ev.value) ? 1:0; 
                    break;
                case KEY_D:
                    iost.p2_drum_3 = (ev.value) ? 1:0; 
                    break;
                case KEY_F:
                    iost.p2_drum_4 = (ev.value) ? 1:0; 
                    break;
                case KEY_G:
                    iost.p2_drum_5 = (ev.value) ? 1:0; 
                    break;
                case KEY_H:
                    iost.p2_drum_6 = (ev.value) ? 1:0; 
                    break;
                default:
                    break;
            }            
        }
    }
}



void keyboardio_init(void){
    pthread_create(&hthread, 0, input_thread, 0);
}