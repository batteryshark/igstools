#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>

#include <sys/mman.h>


#include "song/song_manager.h"
#define PM_SYSTEM_STATUS_SHUTDOWN 4
#define ADDR_SYSTEM_STATUS 0x08429880
#define ADDR_SYSTEM_END 0x08056130
void (*SystemEnd)(void) = (void*)ADDR_SYSTEM_END;

void Shutdown(void){
  SongManager_Stop();
    int* PM_SystemStatus = (int*)ADDR_SYSTEM_STATUS;    
    *PM_SystemStatus = PM_SYSTEM_STATUS_SHUTDOWN;
    SystemEnd();
}

void PrintHex(unsigned char* data, unsigned int len) {
        unsigned int i;
	for (i = 0; i < len; i++) {
		printf("%02X", data[i]);
	}
	printf("\n");
}

void UnprotectPage(int addr){    
    mprotect((void*)(addr-(addr%0x1000)),0x1000,PROT_READ|PROT_WRITE|PROT_EXEC);
}

void PatchCall(void* call_address, void* target_address){
    int call_delta = (int)target_address - (int)(call_address+5);
    UnprotectPage((int)call_address);
    *(int*)(call_address+1) = call_delta;
}

void PatchJump(void* jump_address, void* target_address){
    UnprotectPage((int)jump_address);
    unsigned char jmp_stub[6] = {0x68,0x00,0x00,0x00,0x00,0xC3};
    // Put target address in jmp_stub
    memcpy(jmp_stub+1,&target_address,4);
    // Put the stub onto our target
    memcpy(jump_address,jmp_stub,sizeof(jmp_stub));
}

void msleep(unsigned int num_ms){
    struct timespec ts;
    ts.tv_sec = num_ms / 1000;
    ts.tv_nsec = (num_ms % 1000) * 1000000;
    nanosleep(&ts,NULL);
    
}

long long GetCurrentTimestamp() {
    long long milliseconds;
    struct timeval te; 
    gettimeofday(&te, NULL); // get current time
    milliseconds = te.tv_sec*1000LL + te.tv_usec/1000; // calculate milliseconds    
    return milliseconds;
}

float short_to_float(short val){
    int tens = val / 100;
    int ones = val % 100;
    float outval = (float)tens + (float)ones / 100;
    return outval;
}
