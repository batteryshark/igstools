// PercussionMaster Patch to Emulate A27 for IGS
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>

#include "a27/emu.h"
#include "a27/a27.h"
#include "a27/log.h"
#include "keyio.h"
#include "utils.h"
#include "patches.h"

// TODO: REMOVE THIS
#include "recfile.h"


static int pccard_fake_fd = 0xA271337;
static int keyio_emu = 0;
static int a27_log = 0;
static int a27_emulator = 0;
static off_t a27_log_current_offset = 0;

// PCCARD FileIO Hooks

static int pccard_open(const char * filename, int oflag){
#ifdef A27_EMU_SUPPORTED
    // Test if /dev/pccard0 exists - if not, enable the emulator.
    if(access(PCCARD_PATH,F_OK) < 0){ 
        a27_emulator = 1;
        return pccard_fake_fd;
    }
#endif
    
    return open(filename,oflag);
}

static int pccard_close(int fd){
#ifdef A27_EMU_SUPPORTED
    if(fd == pccard_fake_fd){return 0;}
#endif
    return close(fd);
}

static off_t pccard_lseek(int fd, off_t offset, int whence){
#ifdef A27_EMU_SUPPORTED
    if(fd == pccard_fake_fd){return offset;}
#endif
    if(a27_log){a27_log_current_offset = offset;}    
    return lseek(fd,offset,whence);
}
    
static ssize_t pccard_write(int fd, const void* buf, size_t count){
    // We can't inject into a buffer that doesn't exist!
    if(count && keyio_emu){KeyIO_InjectWrite((unsigned char*)buf);}
    
    // We'll either reset or write our packet.
#ifdef A27_EMU_SUPPORTED
    if(a27_emulator){
        (count) ? A27Emu_Write(buf) : A27Emu_Reset();
        return 1;    
    }
#endif
    // We don't care about logging if it's a fake card.
    if(a27_log){A27Log_Write(a27_log_current_offset,buf,count);}
    
    // Let the real card do its thing.
    return write(fd,buf,count);
}

ssize_t pccard_read(int fd, void *buf, size_t count){
    ssize_t res;
#ifdef A27_EMU_SUPPORTED
    if(a27_emulator){
		// We don't need to add the IO state with our emulator.
		// It's already read during our processing.
        A27Emu_Read(buf);		
        return PCCARD_READ_OK;
    }
#endif

    res = read(fd,buf,count);
	// The real thing, however, only uses keyinput for song judgement
	// and the menus and other switches don't work with it.
    if(keyio_emu){KeyIO_InjectRead((unsigned char*)buf);}
    // We will only log on a real card's data.
    if(a27_log){A27Log_Read(a27_log_current_offset,buf);}
    return res;
}


// --- Entrypoint ---
void __attribute__((constructor)) initialize(void){   
    // TODO: REMOVE THIS
    //load_recfile("/peng/linux/songdata/Cha Cha Queen_Easy.rec",0,0,NULL);
    //char song_path[256] = "./songdata/";
    //GenerateEventFilename(0,3,song_path);
    //printf("Song Path: %s\n",song_path);
    //exit(0);
    // Set up our various patches.
    const char* video_driver;
    if(getenv("SDL_VIDEODRIVER") != NULL){
        video_driver = getenv("SDL_VIDEODRIVER");
        if(!strcmp(video_driver,"dummy")){
            Patch_DummyVideo();
        }
    }
    
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
    if(getenv("PM_TRACKBALL")){Patch_TrackballMenu();}
    if(getenv("PM_AUTOPLAY")){Patch_Autoplay();}
    if(getenv("PM_SKIPWARNING")){Patch_SkipWarning();}
    if(getenv("PM_NEWAUDIO")){Patch_AudioEngine();}
    if(getenv("PM_RECDUMP")){Patch_RecordDump();}
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
