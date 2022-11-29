#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>


static int log_lock = 0;
static char log_path[256] = {0x00};

long long current_timestamp() {
    long long milliseconds;
    struct timeval te; 
    gettimeofday(&te, NULL); // get current time
    milliseconds = te.tv_sec*1000LL + te.tv_usec/1000; // calculate milliseconds    
    return milliseconds;
}

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


void A27Log_Write(off_t offset, const void* buffer, size_t count){
    long long tsm = current_timestamp();
    char flag[] = "logw";
    FILE* fp = open_log_file();
    fwrite(&offset,sizeof(off_t),1,fp);    
    fwrite(flag,4,1,fp);
    fwrite(&tsm,sizeof(tsm),1,fp);    
    fwrite(&count,sizeof(unsigned int),1,fp);
    fwrite(buffer,count,1,fp);    
    close_log_file(fp);
}

void A27Log_Read(off_t offset,void* buffer){
    long long tsm = current_timestamp();
    char flag[] = "logr";
    FILE* fp = open_log_file();    
    size_t count = 132 + *(unsigned int*)buffer;
    fwrite(&offset,sizeof(off_t),1,fp);
    fwrite(flag,4,1,fp);
    fwrite(&tsm,sizeof(tsm),1,fp);    
    fwrite(&count,sizeof(size_t),1,fp);
    fwrite(buffer,count,1,fp);
    close_log_file(fp);
}

void A27Log_init(void){
    const char* lp = getenv("PM_A27LOG");
    char ts_tail[64] = {0x00};
    sprintf(ts_tail,"_%d",(int)time(NULL));
    strcpy(log_path,lp);
    strcat(log_path,ts_tail);
    printf("[A27Log::init] Log file for A27: %s\n",log_path);
}
