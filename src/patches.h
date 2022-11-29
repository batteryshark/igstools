#ifndef __PATCHES_H
#define __PATCHES_H

// Hardcoded Addresses
#define ADDR_SDL_VIDEOFLAGS     0x08056BC8
#define ADDR_SDL_DUMMYVIDEO_FIX 0x08056BE6

#define ADDR_MENU_QCTEST        0x0807508C
#define ADDR_MENU_DEVTEST       0x08076884
#define ADDR_MENU_TRACKBALL     0x08069768
#define ADDR_CALL_OPMENU        0x08056A78

#define ADDR_ENABLE_AUTOPLAY    0x080A7DC7

// Hardcoded Addresses for Hooks
#define ADDR_PCCARD_OPEN        0x08068829
#define ADDR_PCCARD_CLOSE       0x080688D1
#define ADDR_PCCARD_SEEK_INIT   0x080684C7
#define ADDR_PCCARD_WRITE_INIT  0x080684DC
#define ADDR_PCCARD_SEEK_READ   0x08068A17
#define ADDR_PCCARD_SEEK_WRITE  0x08068AD2
#define ADDR_PCCARD_READ        0x08068A2C
#define ADDR_PCCARD_WRITE       0x08068AEE

void Patch_DummyVideo(void);
void Patch_SetWindowedMode(void);


void Patch_QCTest(void);
void Patch_TrackballMenu(void);
void Patch_DevTest(void);
void Patch_Autoplay(void);

#endif
