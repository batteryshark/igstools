// IGS Tools for PercussionMaster (IGSTools)
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <stdbool.h>
#include <dlfcn.h>

#include "a27.h"
#include "pm_keyboardio.h"

#define UNPROTECT(addr,len) (mprotect((void*)(addr-(addr%len)),len,PROT_READ|PROT_WRITE|PROT_EXEC))

#define OFFSET_SDL_VIDEOFLAGS 0x08056BC8
#define OFFSET_SDL_DUMMYVIDEO_FIX 0x08056BE6



static bool a27_logging_enabled = false;
static bool a27_emulator_enabled = false;
static bool keyboard_io_enabled = false;

static int a27_fd = -1;

// Some functions we need to detour
static int (*real_open)(const char *, int) = NULL;
static off_t (*real_lseek)(int fd, off_t offset, int whence) = NULL;
static ssize_t (*real_read)(int fd, void *buf, size_t count) = NULL;
static ssize_t (*real_write)(int fd, const void* buf, size_t count) = NULL;

// libc Wrappers

// Handle A27 Emulation, Do Debug Printing
ssize_t write(int fd, const void* buf, size_t count){
    ssize_t res;
    if (real_write == NULL){
        real_write = dlsym(RTLD_NEXT, "write");
    }
    
    if(fd == a27_fd){
        if(a27_emulator_enabled){
            if(count == 0){
                A27Emu_Reset();
                return 1;
            }
            A27Emu_Process(buf,count);
            return 1;
        }else{
            res = real_write(fd,buf,count);
            if(a27_logging_enabled){
                A27PrintWrite(res, buf, count);
            }
            return res;
        }
    }   
    return real_write(fd,buf,count);
}

ssize_t read(int fd, void *buf, unsigned int count){
    ssize_t res;

    if (real_read == NULL){
        real_read = dlsym(RTLD_NEXT, "read");
    }
    
    if(fd == a27_fd){
        if(a27_emulator_enabled){
            // Read Our Emulated A27 Response
            res = A27Emu_Read(buf);
        }else{
            res = real_read(fd,buf,count);
        }

        if(a27_logging_enabled){
            A27PrintRead(res,buf);
        }

        if(keyboard_io_enabled){
            A27InjectKeyboardIO(buf);
        }

    }else{
        res = real_read(fd,buf,count);
    }

    return res;
}

// This is pretty much for logging what that seek is doing to the card.
off_t lseek(int fd, off_t offset, int whence){
    off_t res;

    if(real_lseek == NULL){
        real_lseek = dlsym(RTLD_NEXT, "lseek");
    }
    
    res = real_lseek(fd,offset,whence);

    if(fd == a27_fd && a27_logging_enabled){
        printf("[A27Log] Seek: Offset %ld, Whence: %d, Res: %ld\n",offset,whence,res);
    }
    
    return res;
}

// If we're accessing the PCCARD Device, give a fake fd if it's emulated. If not, record the real one and passthrough.
int open(const char * filename, int oflag){
    if (real_open == NULL){
        real_open = dlsym(RTLD_NEXT, "open");
    }
    
    if(!strcmp(filename, PCCARD_PATH)){
        if(a27_emulator_enabled){return a27_fd;}
        a27_fd = real_open(filename,oflag);
        if(a27_logging_enabled){
            printf("[A27Log] Opened PCCARD: %d\n",a27_fd);
        }
        return a27_fd;
    }
    return real_open(filename, oflag);
}


void init(){

    // Check if the fullscreen envar has been set - patch to windowed mode, otherwise.
    if(!getenv("PM_FULLSCREEN")){
        printf("[IGSTools::init] Setting Game to Windowed Mode.\n");
        UNPROTECT(OFFSET_SDL_VIDEOFLAGS, 4096);
        *((char*)OFFSET_SDL_VIDEOFLAGS) = 0x00;
    }

    // Check if we're disabling the video rendering - patch out the error.
    if(getenv("SDL_VIDEODRIVER") != NULL){
        const char* vdriver = getenv("SDL_VIDEODRIVER");
        if(!strcmp(vdriver,"dummy")){
            printf("[IGSTools::init] Setting Fix for SDL dummy Video.\n");
            UNPROTECT(OFFSET_SDL_DUMMYVIDEO_FIX, 4096);
            *((char*)OFFSET_SDL_DUMMYVIDEO_FIX) = 0xEB;
        }
    }

    // Check for IGS PCCARD - via /dev/pccard0 - if not, enable io and a27 emulation.
    if(access(PCCARD_PATH,F_OK) < 0){
        a27_emulator_enabled = true;
        keyboard_io_enabled = true;
    }

    // Check if PM_KEYBOARDIO is set - if so, enable io emulation.
    if(getenv("PM_KEYBOARDIO")){
        keyboard_io_enabled = true;
    }

    // Check if PM_A27LOGGING is set - if so, enable a27 logging mode.
    if(getenv("PM_A27LOGGING")){
        a27_logging_enabled = true;
    } 

    printf("[IGSTools::init] OPTIONS <[A27Emulation: %d, KeyboardIO: %d, A27Logging: %d]>\n",a27_emulator_enabled,keyboard_io_enabled,a27_logging_enabled);

    if(keyboard_io_enabled){
        keyboardio_init();
    }

    if(a27_emulator_enabled){
        a27_fd = FAKE_PCCARD_FD;
    }
}

void __attribute__((constructor)) initialize(void){
    printf("[IGSTools::Startup] Starting up IGSTools...\n");
    init();
}