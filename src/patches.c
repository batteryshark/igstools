// Various Binary Patches for PercussionMaster
#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "utils.h"
#include "soundmgr.h"

#include "patches.h"

// MTV Localization Stuff
struct estr_entry{
    int addr;
    char* str;
};

static char* noyes[] = {
    "No",
    "Yes"
};

static char* yesno[] = {
    "Yes",
    "No"
};

static char* speed_options[] = {
    "Normal",
    "S1x",
    "S2x",
    "S3x",
    "S4x"
};

static char* cloak_options[] = {
    "Normal",
    "1x",
    "2x",
    "3x"
};

static char* record_options[] = {
    "None",
    "Record",
    "Play"
};

static char* judgement_options[] = {
    "Easy",
    "Normal",
    "Hard",
    "Expert"
};

static char* difficulty_options[] = {
    "Training",
    "Elementary",
    "Intermediate",
    "Advanced",
    "Super Advanced",
    "Challenge",
    "Drum King",
    "END",
    "Battle"
};

static struct estr_entry test_menu_english[] = {
    {0x0807B1BD,"[%d:%2d] Notes:%d Tempo:%d Moves:%d"},
    {0x0807B1EA,"Stars: %02d SecID: %d Difficulty: %s"},
    {0x0807B202,"Select Song"},
    {0x0807B22B,(char*)difficulty_options},
    {0x0807B230,"Mode: %s"},
    {0x0807B256,(char*)noyes},
    {0x0807B265,"%dP Enabled: %s"},
    {0x0807B2AD,"%dP Autoplay: %s"},
    {0x0807B2D3,(char*)speed_options},
    {0x0807B2DF,"%dP Speed Mod: %s"},
    {0x0807B308,(char*)cloak_options},
    {0x0807B30E,"%dP Cloak Mod: %s"},
    {0x0807B334,"%dP Noteskin: %d"},
    {0x0807B359,(char*)record_options},
    {0x0807B35E,"KeyRecord: %s"},
    {0x0807B386,"File Index: %d"},
    {0x0807B3AF,(char*)yesno},
    {0x0807B3B4,"Keysounds: %s"},
    {0x0807B3E0,"Player Vol: %d"},
    {0x0807B40C,"Song Vol: %d"},
    {0x0807B438,"BG Vol: %d"},
    {0x0807B461,(char*)judgement_options},
    {0x0807B466,"Judgement: %s"},
};

// Localize the Song Test Menu 
void Localize_MTVMenu(void){
    int num_str_entries;
    int curr_entry;
    void** cur_addr;
    num_str_entries = sizeof(test_menu_english) / sizeof(struct estr_entry);
    for(curr_entry=0; curr_entry < num_str_entries; curr_entry++){
        UnprotectPage(test_menu_english[curr_entry].addr);
        cur_addr = (void*)test_menu_english[curr_entry].addr;
        *cur_addr = test_menu_english[curr_entry].str;
    }    
}

// Replace call to operator menu with the menu given.
void Replace_OperatorMenu(int replacement_menu){
    UnprotectPage(ADDR_CALL_OPMENU);
    *(int*)(ADDR_CALL_OPMENU+1) = (replacement_menu - (int)(ADDR_CALL_OPMENU + 5));
}


// -- Exports --

void Patch_SetWindowedMode(void){
    printf("[Patches::Windowed] Setting Game to Windowed Mode.\n");
    UnprotectPage(ADDR_SDL_VIDEOFLAGS);
    *(char*)ADDR_SDL_VIDEOFLAGS = 0;
}

void Patch_DummyVideo(void){
    printf("[Patches::DummyVideo] Setting Fix for SDL dummy Video.\n");
    UnprotectPage(ADDR_SDL_DUMMYVIDEO_FIX);
    *(char*)ADDR_SDL_DUMMYVIDEO_FIX = 0xEB;
}

void Patch_QCTest(void){
    printf("[Patches::QCTest] Enabling QCTest Menu.\n");
    Replace_OperatorMenu(ADDR_MENU_QCTEST);
}

void Patch_TrackballMenu(void){
    printf("[Patches::Autoplay] Enabling Trackball Menu.\n");
    Replace_OperatorMenu(ADDR_MENU_TRACKBALL);
}

void Patch_DevTest(void){
    printf("[Patches::DevTest] Enabling DevTest Menu.\n");
    Replace_OperatorMenu(ADDR_MENU_DEVTEST);
    Localize_MTVMenu();
}

void Patch_Autoplay(void){
    printf("[Patches::Autoplay] Enabling Autoplay.\n");
    UnprotectPage(ADDR_ENABLE_AUTOPLAY);
    *(char*)ADDR_ENABLE_AUTOPLAY = 1;
}

void Patch_SkipWarning(void){
        printf("[Patches::Warning] Skip Warning.\n");
        UnprotectPage(ADDR_SKIP_WARNING);
        UnprotectPage(ADDR_SKIP_WARNING2);
        *(unsigned int*)ADDR_SKIP_WARNING = 10;
        *(unsigned int*)ADDR_SKIP_WARNING2 = 10;
}



void Patch_AudioEngine(void){

 printf("[Patches::AudioEngine] Using new FMOD Engine\n");
 // Patch MixerInit to Always Return 1
 UnprotectPage(0x08067E30);
 *(unsigned int*)0x08067E30 = 0xC340C031;
 
 // Patch MusicInit to just return.
 UnprotectPage(0x08064D3C);
 *(unsigned char*)0x08064D3C = 0xC3;
 
 // Patch SoundInit to use our SoundMgrInit
 PatchCall((void*)0x080560B9,SoundMgr_Init);
 
 // Patch SoundLoad/SoundLoadDynamic to use our own SoundLoad
 PatchJump((void*)0x08065430,SoundMgr_Load);
 PatchJump((void*)0x08065488,SoundMgr_Load);
 
 // Patch Load /  Credit Sound to use our own
 PatchJump((void*)0x08065AC0,SoundMgr_LoadCredit);
 PatchJump((void*)0x08065B88,SoundMgr_PlayCredit);
 
 
 
 // Patch SoundPlay to use our own
 PatchJump((void*)0x08065524,SoundMgr_Play);
 
 // Patch SoundStop to use our own
 PatchJump((void*)0x0806563C, SoundMgr_Stop);
 
 // Patch SoundFree and SoundFreeAll to call our own
 PatchJump((void*)0x080656A4,SoundMgr_Unload);
 PatchJump((void*)0x08065750,SoundMgr_UnloadAll);
 
 // SoundProcess we'll change to call our update tick
 PatchJump((void*)0x080657D0,SoundMgr_Update);
 
 // Patch SoundEnd to inline our own shutdown function.
 PatchJump((void*)0x08065790,SoundMgr_Shutdown);
 
 // Patch MusicEnd to just return
 UnprotectPage(0x0806516C);
 *(unsigned char*)0x0806516C = 0xC3;
 
}
