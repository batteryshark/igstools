#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <unistd.h>
#include <signal.h>
#include <execinfo.h>
#ifdef A27_EMU_SUPPORTED
#include "../a27/song/song_manager.h"
#endif
// -- Debug Functions
void PrintHex(unsigned char* data, unsigned int len) {
    for (int i = 0; i < len; i++) {
        printf("%02X", data[i]);
    }
    printf("\n");
}

void PrintBacktrace(void){
        void *array[64];
    unsigned long size;

    size = backtrace(array, 64);
    backtrace_symbols_fd(array, size, STDERR_FILENO);
}

// -- Process Functions
#define PM_SYSTEM_STATUS_SHUTDOWN 4
#define ADDR_SYSTEM_STATUS 0x08429880
#define ADDR_SYSTEM_END 0x08056130
void (*SystemEnd)(void) = (void*)ADDR_SYSTEM_END;

void Shutdown(void){
  
    int* PM_SystemStatus = (int*)ADDR_SYSTEM_STATUS;    
    *PM_SystemStatus = PM_SYSTEM_STATUS_SHUTDOWN;
    SystemEnd();
}
void QuitProcess(void){
#ifdef A27_EMU_SUPPORTED
    SongManager_StopSong();
#endif
    Shutdown();
    exit(0);
    kill(getpid(),SIGKILL);
}

// -- Patching Functions
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

// -- Time-Based Functions

// Sleep a specified number of milliseconds.
void SleepMS(unsigned int num_ms){
    struct timespec ts;
    ts.tv_sec = num_ms / 1000;
    ts.tv_nsec = (num_ms % 1000) * 1000000;
    nanosleep(&ts,NULL);
    
}

// Get a Timestamp in Milliseconds
long long GetCurrentTimestamp() {
    long long milliseconds;
    struct timeval te; 
    gettimeofday(&te, NULL); // get current time
    milliseconds = te.tv_sec*1000LL + te.tv_usec/1000; // calculate milliseconds    
    return milliseconds;
}

// Given a start time, current time, target time, start position, and end position, derive what position the target is currently in.
unsigned short DeriveDistanceByTime(long start_time, long current_time, long end_time, long start_position, long end_position){
    // Get total life of the target.
    long total_lifetime = end_time - start_time;
    // Get how much time has already passed.
    long lifetime_progress = current_time - start_time;
    // Build a ratio to get a percentage for progress.
    float progress_ratio = (float)lifetime_progress/(float)total_lifetime;
    // Get how much distance we expect to cover given lifetime.
    long total_distance = end_position - start_position;
    // Get how much distance we have currently covered.
    float fdistance = (float)total_distance * progress_ratio;
    // Return the amount of distance + the start position in case start isn't zero.
    return (unsigned short)start_position + fdistance;
}

// -- Conversion Functions
float Short2Float(short val){
    int tens = val / 100;
    int ones = val % 100;
    float outval = (float)tens + (float)ones / 100;
    return outval;
}
