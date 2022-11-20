// IGS PCCARD w/a27 Driver

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dlfcn.h>

#include "../shared/a27.h"

#define A27_FAKE_HANDLE 0x0A271337
static off_t a27_fops_counter = 0;

static struct A27_Read_Message a27_state = {0};

static int (*real_open)(const char *, int) = NULL;
static off_t (*real_lseek)(int fd, off_t offset, int whence) = NULL;
static ssize_t (*real_read)(int fd, void *buf, size_t count) = NULL;
static ssize_t (*real_write)(int fd, void *buf, size_t count) = NULL;



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

void a27_set_checksum(void){
    unsigned int cval = (a27_state.a27_has_message & 0xFF) + \
        (a27_state.inet_password_data & 0xFF) + \
        a27_state.is_light_io_reset + \
        (a27_state.asic_errnum & 0xFF) + \
        a27_state.asic_iserror + \
        a27_state.coin_inserted + \
        (a27_state.dwBufferSize & 0xFF) + \
        (a27_state.system_mode & 0xFF);
    a27_state.checksum_1 = (cval & 0xFF);
    a27_state.checksum_2 = (cval & 0xFF);

}

void A27_Reset(){
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
int ctr = 0;
void a27_process_song(unsigned char* data, unsigned int length){
    unsigned int code = *(unsigned int*)data;
    switch(code){
        case 3:
            printf("Case 3\n");
            memset(a27_state.data,0,sizeof(a27_state.data));
            a27_state.data[0] = 3;
            a27_state.dwBufferSize = 0xA00;
             
            break;
        case 4:
            printf("Case 4\n");
            memset(a27_state.data,0,sizeof(a27_state.data));
            a27_state.data[0] = 4;
            a27_state.dwBufferSize = 0xA00;          
            break;
        case 5:
        case 6:     
      
            a27_state.data[0] = 6;
            a27_state.data[4] = 0x02;
            a27_state.data[5] = 0x00;
            a27_state.data[6] = 0x02;
            a27_state.data[7] = 0x00;        
            ctr++;
            if(ctr > 0xFF){ctr = 0;}
            a27_state.data[0x48] = 0x02;
            a27_state.data[0x49] = 0x40;
            a27_state.data[0x4A] = 0x40;
            a27_state.data[0x4C] = ctr % 0xFF;
            a27_state.data[0x4F8] = 0x02;
            a27_state.data[0x4F9] = 0x40;
            a27_state.data[0x4FA] = 0x40;
            a27_state.data[0x4FC] = ctr % 0xFF;                           
            a27_state.data[0x9B0] = 0x01;
            a27_state.data[0x9B1] = 0x01;
            a27_state.data[0x9F8] = 0x0A;
            a27_state.data[0x9FA] = 0x0A;
            a27_state.dwBufferSize = 0;               
            break;
        case 9:
            memset(a27_state.data,0,sizeof(a27_state.data));
            a27_state.data[0] = 9;
            a27_state.dwBufferSize = 80;             
            a27_state.data[76] = 3;
            a27_state.data[77] = 3;
            a27_state.data[78] = 2;
            a27_state.data[79] = 2;
            break;
        default:
            printf("Unknown Opcode for Song: %d\n",code);
            print_hex(data,length);
            exit(-1);
    }
}

void a27_process_request(unsigned char* data, size_t count){
    struct A27_Write_Message* req = (struct A27_Write_Message*)data;
    
    switch(req->system_mode){
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
            return;
        case 27:
            A27_Reset();
            break;            
        case A27_MODE_SONG:
            //printf("[Song]: ");
            //print_hex(data,count);
            a27_process_song(req->data,req->dwBufferSize);
            break;
        default:
            printf("Unhandled Operation: %d\n",req->system_mode);
            printf("System Mode: %d\n",req->system_mode);
            if(req->dwBufferSize){
                printf("Payload Size: %d\n", req->dwBufferSize);
                print_hex(req->data,req->dwBufferSize);
            }            
            exit(0);
    }
    
}

ssize_t a27_write_operation(int fd, void* buf, size_t count){
    if(count == 0 && a27_fops_counter == 254){
        A27_Reset();
        return 1;
    }
    a27_process_request(buf,count);
    return 1;
}

int a27_read_operation(int fd, void* buf, size_t count){
    a27_set_checksum();    
    memcpy((unsigned char*)buf,&a27_state,A27_Response_Header_Size);
    memcpy((unsigned char*)buf+A27_Response_Header_Size,a27_state.data,a27_state.dwBufferSize);
    return PCCARD_READ_OK;    
}


off_t lseek(int fd, off_t offset, int whence){
    if (real_lseek == NULL){
      real_lseek = dlsym(RTLD_NEXT, "lseek");
    }
    if(fd == A27_FAKE_HANDLE){
        a27_fops_counter = offset;
        return a27_fops_counter;
    }

    return real_lseek(fd,offset,whence);
}

// Basically, when you write to the pccard and it's fops 254 with 0 bytes to write, you're gonna call A27_Reset
ssize_t write(int fd, unsigned char *buf, size_t count){
    if (real_write == NULL){
      real_write = dlsym(RTLD_NEXT, "write");
    }
    
    if(fd == A27_FAKE_HANDLE)
        return a27_write_operation(fd,buf,count);
    
    return real_write(fd,buf,count);
}

ssize_t read(int fd, void *buf, size_t count){    
    if (real_read == NULL){
      real_read = dlsym(RTLD_NEXT, "read");
    }
 
    if(fd == A27_FAKE_HANDLE)
        return a27_read_operation(fd,buf,count);
    
    return real_read(fd,buf,count);
}

void write_packet(unsigned char system_mode, char* hex_data){
	struct A27_Write_Message* wm = malloc(sizeof(struct A27_Write_Message));
    size_t length = strlen(hex_data)+1;
	size_t i;
	size_t j;
	for (i = 0, j = 0; i < (length / 2); i++, j += 2){
		wm->data[i] = (hex_data[j] % 32 + 9) % 25 * 16 + (hex_data[j+1] % 32 + 9) % 25;
	}
	wm->dwBufferSize = i;
    wm->system_mode = system_mode;
    printf("Write Packet Mode: %d\n",system_mode);
    printf("Payload Length: %d Bytes\n",wm->dwBufferSize);
    print_hex(wm->data,wm->dwBufferSize);
    exit(1);

}



int open(const char * filename, int oflag){
    
    if (real_open == NULL){
      real_open = dlsym(RTLD_NEXT, "open");
    }
    
    if (strcmp(filename, PCCARD_PATH) == 0)
        return A27_FAKE_HANDLE;

    return real_open(filename, oflag);
}
