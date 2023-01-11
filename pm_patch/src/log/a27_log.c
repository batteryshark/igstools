// Logging Component for A27 Packet Recording
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../utils/utils.h"

static int log_lock = 0;
static char log_path[256] = {0x00};
static off_t a27_packet_id = 0;

FILE* open_log_file(){
    FILE* fp;
    while(log_lock){}
    log_lock = 1;
    fp = fopen(log_path,"ab");
    if(!fp){
        printf("Open Log File Error occurred: %s!\n",log_path);
        exit(-1);
    }
    return fp;
}

void close_log_file(FILE* fp){
    fclose(fp);
    log_lock = 0;
}

void A27Log_PacketId(off_t value){
    a27_packet_id = value;
}

void A27Log_Write(const void* buffer, size_t count){
    long long tsm = GetCurrentTimestamp();
    char flag[] = "logw";
    FILE* fp = open_log_file();
    fwrite(&a27_packet_id,sizeof(off_t),1,fp);    
    fwrite(flag,4,1,fp);
    fwrite(&tsm,sizeof(tsm),1,fp);    
    fwrite(&count,sizeof(unsigned int),1,fp);
    fwrite(buffer,count,1,fp);    
    close_log_file(fp);
}

void A27Log_Read(void* buffer){
    long long tsm = GetCurrentTimestamp();
    char flag[] = "logr";
    FILE* fp = open_log_file();    
    size_t count = 132 + *(unsigned int*)buffer;
    fwrite(&a27_packet_id,sizeof(off_t),1,fp);
    fwrite(flag,4,1,fp);
    fwrite(&tsm,sizeof(tsm),1,fp);    
    fwrite(&count,sizeof(size_t),1,fp);
    fwrite(buffer,count,1,fp);
    close_log_file(fp);
}

void A27Log_init(void){
    const char* lp = getenv("PM_A27LOG");
    char ts_tail[64] = {0x00};
    sprintf(ts_tail,"_%llu",GetCurrentTimestamp());
    strcpy(log_path,lp);
    strcat(log_path,ts_tail);
    printf("[A27Log::init] Log file for A27: %s\n",log_path);
}
