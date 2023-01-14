// PercussionMaster Patch to Emulate A27 for IGS
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>

#include "a27/a27.h"
#include "log/a27_log.h"
#include "io/keyio.h"
#include "patches/patches.h"
#include "utils/utils.h"

#ifdef A27_EMU_SUPPORTED
    #include "a27/emulator.h"
#endif

// Hardcoded Addresses for Hooks
#define ADDR_PCCARD_OPEN        0x08068829
#define ADDR_PCCARD_CLOSE       0x080688D1
#define ADDR_PCCARD_SEEK_INIT   0x080684C7
#define ADDR_PCCARD_WRITE_INIT  0x080684DC
#define ADDR_PCCARD_SEEK_READ   0x08068A17
#define ADDR_PCCARD_SEEK_WRITE  0x08068AD2
#define ADDR_PCCARD_READ        0x08068A2C
#define ADDR_PCCARD_WRITE       0x08068AEE

static int keyio_emu = 0;
static int a27_log = 0;
static int a27_emulator = 0;

// PCCARD FileIO Hooks
static int pccard_open(const char * filename, int oflag){
#ifdef A27_EMU_SUPPORTED
    // Test if /dev/pccard0 exists - if not, enable the emulator.
    if(access(PCCARD_PATH,F_OK) < 0){ 
        a27_emulator = 1;
        return PCCARD_FAKE_FD;
    }
#endif
    return open(filename,oflag);
}

static int pccard_close(int fd){
#ifdef A27_EMU_SUPPORTED
    if(fd == PCCARD_FAKE_FD){return 0;}
#endif
    return close(fd);
}

static off_t pccard_lseek(int fd, off_t offset, int whence){
#ifdef A27_EMU_SUPPORTED
    if(fd == PCCARD_FAKE_FD){return offset;}
#endif
    if(a27_log){A27Log_PacketId(offset);}    
    return lseek(fd,offset,whence);
}
    
static ssize_t pccard_write(int fd, const void* buf, size_t count){
    // We can't inject into a buffer that doesn't exist!
    if(count && keyio_emu){KeyIO_InjectWrite((PA27WriteHeader)buf);A27SetWriteChecksum((PA27WriteHeader)buf);}
    
    // We'll either reset or write our packet.
#ifdef A27_EMU_SUPPORTED
    if(a27_emulator){
        (count) ? A27Emu_Write((PA27WriteMessage)buf) : A27Emu_Reset();
        return 1;    
    }
#endif
    // We don't care about logging if it's a fake card.
    if(a27_log){A27Log_Write(buf,count);}
    
    // Let the real card do its thing.
    return write(fd,buf,count);
}

ssize_t pccard_read(int fd, void *buf, size_t count){
    ssize_t res;
#ifdef A27_EMU_SUPPORTED
    if(a27_emulator){
		// We don't need to add the IO state with our emulator.
		// It's already read during our processing.
        A27Emu_Read((PA27ReadMessage)buf);
        return PCCARD_READ_OK;
    }
#endif

    res = read(fd,buf,count);
	// The real thing, however, only uses keyinput for song judgement
	// and the menus and other switches don't work with it.
    if(keyio_emu){KeyIO_InjectRead((PA27ReadHeader)buf);A27SetReadChecksum((PA27ReadHeader)buf);}
    // We will only log on a real card's data.
    if(a27_log){A27Log_Read(buf);}
    return res;
}


// --- Entrypoint ---
void __attribute__((constructor)) initialize(void){       
    
    // We'll always patch the filesystem paths to not be absolute [damn sith].
    Patch_FilesystemPaths();
    // We'll also always patch the hang that happens on staff audio.
    Patch_OSSSoundFix();
    // Set up our various 'optional' patches.
    if(getenv("PM_WINDOWED")){Patch_SetWindowedMode();}
    
    if(getenv("PM_KEYIO")){
        keyio_emu = 1;
        KeyIO_Init();
    }
    
    if(getenv("PM_A27LOG")){
        a27_log = 1;
        A27Log_init();
    }
    
    if(getenv("PM_QCTEST")){Patch_QCTest();}
    if(getenv("PM_DEVTEST")){Patch_DevTest();}
    if(getenv("PM_TRACKBALLTEST")){Patch_TrackballMenu();}
    if(getenv("PM_AUTOPLAY")){Patch_Autoplay();}
    if(getenv("PM_SKIPWARNING")){Patch_SkipWarning();}
    if(getenv("PM_LANGUAGE")){Patch_Language();}

    // Universal pccard fileio hooks
    PatchCall((void*)ADDR_PCCARD_OPEN,(void*)pccard_open);
    PatchCall((void*)ADDR_PCCARD_CLOSE,(void*)pccard_close);
    PatchCall((void*)ADDR_PCCARD_SEEK_INIT,(void*)pccard_lseek);
    PatchCall((void*)ADDR_PCCARD_WRITE_INIT,(void*)pccard_write);
    PatchCall((void*)ADDR_PCCARD_SEEK_READ,(void*)pccard_lseek);
    PatchCall((void*)ADDR_PCCARD_SEEK_WRITE,(void*)pccard_lseek);
    PatchCall((void*)ADDR_PCCARD_READ,(void*)pccard_read);
    PatchCall((void*)ADDR_PCCARD_WRITE,(void*)pccard_write);
    
}
