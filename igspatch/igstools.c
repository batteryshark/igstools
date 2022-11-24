// IGS Tools for PercussionMaster (IGSTools)
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <dlfcn.h>

#define OFFSET_SDL_VIDEOFLAGS     0x08056BC8
#define OFFSET_SDL_DUMMYVIDEO_FIX 0x08056BE6
#define PCCARD_PATH "/dev/pccard0"
#define UNPROTECT(addr,len) (mprotect((void*)(addr-(addr%len)),len,PROT_READ|PROT_WRITE|PROT_EXEC))

const char* key_io_path = "./a27_keyio.so";
const char* xkey_io_path = "./a27_xkeyio.so";
// Some functions we need to detour
static int (*real_open)(const char *, int) = NULL;
static off_t (*real_lseek)(int fd, off_t offset, int whence) = NULL;
static ssize_t (*real_read)(int fd, void *buf, size_t count) = NULL;
static ssize_t (*real_write)(int fd, const void* buf, size_t count) = NULL;


// A27Emu Bindings
static int (*A27Emu_Open)(void) = NULL;
static int (*A27Emu_Read)(void* buf) = NULL;
static int (*A27Emu_Write)(const void* buf,size_t count) = NULL;

// A27Log Bindings
static void (*A27Log_Seek)(int offset, int result) = NULL;
static void (*A27Log_Read)(int result, void* buffer) = NULL;
static void (*A27Log_Write)(int result, unsigned int count, const void* buffer) = NULL;
static void (*A27Log_init)(void) = NULL;

// A27KeyIO Bindings
static void (*A27KeyIO_Inject)(void* data) = NULL;
static void (*A27KeyIO_Init)(void) = NULL;


// Some Static Locals
static unsigned char a27_logging_enabled = 0;
static unsigned char a27_emulator_enabled = 0;
static unsigned char a27_keyio_enabled = 0;
static int a27_fd = -1;

// External Library Initialization
void loadlib_A27Emu(void){
    void* a27emu_lib = dlopen("./a27_emu.so",RTLD_NOW);
    printf("[IGSTools::init] Enabling A27 Emulator...\n");
    if(!a27emu_lib){
        printf("[IGSTools::loadlib_A27Emu] Error: Could not Open library.\n");
        exit(-1);
    }
    A27Emu_Open = dlsym(a27emu_lib,"A27Emu_Open");
    A27Emu_Read = dlsym(a27emu_lib,"A27Emu_Read");
    A27Emu_Write = dlsym(a27emu_lib,"A27Emu_Write");

    if(!A27Emu_Open || !A27Emu_Read || !A27Emu_Write){
        printf("[IGSTools::loadlib_A27Emu] Error: Could not bind to functions.\n");
        exit(-1);
    }
    a27_emulator_enabled = 1;  
}

void loadlib_A27Log(void){
    void* a27log_lib = dlopen("./a27_log.so",RTLD_NOW);
    printf("[IGSTools::init] Enabling A27 Logging...\n");
    if(!a27log_lib){
        printf("[IGSTools::loadlib_A27Log] Error: Could not Open library.\n");
        exit(-1);
    }
    A27Log_Seek = dlsym(a27log_lib,"A27Log_Seek");
    A27Log_Read = dlsym(a27log_lib,"A27Log_Read");
    A27Log_Write = dlsym(a27log_lib,"A27Log_Write");
    A27Log_init = dlsym(a27log_lib,"A27Log_init");
    if(!A27Log_init || !A27Log_Seek || !A27Log_Read || !A27Log_Write){
        printf("[IGSTools::loadlib_A27Log] Error: Could not bind to functions.\n");
        exit(-1);
    }
    a27_logging_enabled = 1;
    A27Log_init();
}

void loadlib_A27KeyIO(const char* keyio_path){
    void* a27keyio_lib = dlopen(keyio_path,RTLD_NOW);
    printf("[IGSTools::init] Enabling A27 KeyboardIO...\n");
    if(!a27keyio_lib){
        printf("[IGSTools::loadlib_A27KeyIO] Error: Could not Open library.\n");
        exit(-1);
    }

    A27KeyIO_Inject = dlsym(a27keyio_lib,"A27KeyIO_Inject");
    A27KeyIO_Init = dlsym(a27keyio_lib,"A27KeyIO_Init");
    if(!A27KeyIO_Inject){
        printf("[IGSTools::loadlib_A27KeyIO] Error: Could not bind to functions.\n");
        exit(-1);
    }
    a27_keyio_enabled = 1;
    A27KeyIO_Init();
}


// --- libc Bindings ---

ssize_t write(int fd, const void* buf, size_t count){
    ssize_t res;

    if (real_write == NULL){
        real_write = dlsym(RTLD_NEXT, "write");
    }
    
    if(fd == a27_fd){
        if(a27_emulator_enabled){
            res = A27Emu_Write(buf,count);
        }else{
            res = real_write(fd,buf,count);
        }

        if(a27_logging_enabled){
            A27Log_Write(res,count,buf);
        }
    
    }else{
        res = real_write(fd,buf,count);        
    }

    return res;
}

ssize_t read(int fd, void *buf, size_t count){
    ssize_t res;

    if (real_read == NULL){
        real_read = dlsym(RTLD_NEXT, "read");
    }
    
    if(fd == a27_fd){
        if(a27_emulator_enabled){
            res = A27Emu_Read(buf);
        }else{
            res = real_read(fd,buf,count);
        }

        if(a27_logging_enabled){
            A27Log_Read(res,buf);
        }

        if(a27_keyio_enabled){
            A27KeyIO_Inject(buf);
        }

    }else{
        res = real_read(fd,buf,count);
    }

    return res;
}

off_t lseek(int fd, off_t offset, int whence){
    off_t res;

    if(real_lseek == NULL){
        real_lseek = dlsym(RTLD_NEXT, "lseek");
    }
    
    res = real_lseek(fd,offset,whence);

    if(fd == a27_fd && a27_logging_enabled){
        A27Log_Seek(offset,res);
    }
    
    return res;
}

// If we're accessing the PCCARD Device, give a fake fd if it's emulated. If not, record the real one and passthrough.
int open(const char * filename, int oflag){
    if (real_open == NULL){
        real_open = dlsym(RTLD_NEXT, "open");
    }
    
    if(!strcmp(filename, PCCARD_PATH)){
        if(a27_emulator_enabled){
            a27_fd = A27Emu_Open();            
        }else{
            a27_fd = real_open(filename,oflag);
        }        
        return a27_fd;
    }
    return real_open(filename, oflag);
}

// --- Entrypoint ---
void __attribute__((constructor)) initialize(void){
    const char* vdriver;
    printf("[IGSTools::Startup] Starting up IGSTools ...\n");

    // Patch game to windowed mode.
    if(getenv("PM_WINDOWED")){
        printf("[IGSTools::init] Setting Game to Windowed Mode.\n");
        UNPROTECT(OFFSET_SDL_VIDEOFLAGS, 4096);
        *((char*)OFFSET_SDL_VIDEOFLAGS) = 0x00;
    }

    // If we're disabling the video rendering - patch out the error.
    if(getenv("SDL_VIDEODRIVER") != NULL){
        vdriver = getenv("SDL_VIDEODRIVER");
        if(!strcmp(vdriver,"dummy")){
            printf("[IGSTools::init] Setting Fix for SDL dummy Video.\n");
            UNPROTECT(OFFSET_SDL_DUMMYVIDEO_FIX, 4096);
            *((char*)OFFSET_SDL_DUMMYVIDEO_FIX) = 0xEB;
        }
    }

    // Check for IGS PCCARD. If not available, enable a27 emulation.
    if(access(PCCARD_PATH,F_OK) < 0){        
        loadlib_A27Emu();
    }

    // Check if PM_KEYBOARDIO is set - if so, enable io emulation.
    if(getenv("PM_KEYBOARDIO")){
        loadlib_A27KeyIO(key_io_path);
    }

    if(getenv("PM_XKEYBOARDIO")){
        loadlib_A27KeyIO(xkey_io_path);       
    }

    // Check if PM_A27LOG is set - if so, enable a27 logging mode.
    if(getenv("PM_A27LOG")){
        loadlib_A27Log();
    }

    if(getenv("PM_QCTEST")){
        UNPROTECT(0x8056A3B,4096);
        memset((void*)0x8056A3B,0x90,0x24);
    }

    if(getenv("PM_AUTOPLAY")){
        UNPROTECT(0x80A7DDE,4096);
        memset((void*)0x80A7DDE,0x90,0x68);
    }

   
}

