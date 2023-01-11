// Various Binary Patches for PercussionMaster
#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "../utils/utils.h"
#include "patches.h"

// Hardcoded Addresses for Patches
#define ADDR_SDL_VIDEOFLAGS     0x08056BC8
#define ADDR_MENU_QCTEST        0x0807508C
#define ADDR_MENU_DEVTEST       0x08076884
#define ADDR_MENU_TRACKBALL     0x08069768
#define ADDR_CALL_OPMENU        0x08056A78
#define ADDR_SKIP_WARNING       0x080A7D04
#define ADDR_SKIP_WARNING2      0x080A7DBD
#define ADDR_ENABLE_AUTOPLAY    0x080A7DC7


// MTV Localization Stuff
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

static StrReplaceEntry test_menu_english[] = {
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

// Linux Path Replacements
static StrReplaceEntry linux_path_entries[] = {
    {0x08172040,"./band1/rom/chainl.rom"},
    {0x08172140,"./band1/rom/effect3l.rom"},
    {0x08172240,"./band1/rom/churchl.rom"},
    {0x08172340,"./band1/rom/effect3l.rom"},
    {0x08172440,"./band1/rom/starnightl.rom"},
    {0x08172540,"./band1/rom/effect2l.rom"},
    {0x08172640,"./band1/rom/herol.rom"},
    {0x08172740,"./band1/rom/effect1l.rom"},
    {0x08172840,"./band1/rom/punkl.rom"},
    {0x08172940,"./band1/rom/effect1l.rom"},
    {0x08172A40,"./band1/rom/taiwangol.rom"},
    {0x08172B40,"./band1/rom/effect3l.rom"},
    {0x08172C40,"./band1/rom/lovewordl.rom"},
    {0x08172D40,"./band1/rom/effect2l.rom"},
    {0x08172E40,"./band1/rom/ancientl.rom"},
    {0x08172F40,"./band1/rom/effect1l.rom"},
    {0x08173040,"./band1/rom/ninjal.rom"},
    {0x08173140,"./band1/rom/effect2l.rom"},
    {0x08173240,"./band1/rom/spacel.rom"},
    {0x08173340,"./band1/rom/effect1l.rom"},
    {0x08173440,"./band1/rom/baseballl.rom"},
    {0x08173540,"./band1/rom/effect3l.rom"},
    {0x08173640,"./band1/rom/summerl.rom"},
    {0x08173740,"./band1/rom/effect3l.rom"},
    {0x08173840,"./band1/rom/bigcityl.rom"},
    {0x08173940,"./band1/rom/effect3l.rom"},
    {0x08173A40,"./band1/rom/wildwestl.rom"},
    {0x08173B40,"./band1/rom/effect2l.rom"},
    {0x081200B7,"./main/selmodel.rom"},
    {0x081200D2,"./main/rolel.rom"},
    {0x081200EA,"./main/role1l.rom"},
    {0x08120103,"./main/role2l.rom"},
    {0x0812011C,"./main/role3l.rom"},
    {0x08120135,"./main/role4l.rom"},
    {0x081201EF,"./main/coinpagel.rom"},
    {0x08120325,"./main/photol.rom"},
    {0x0812073C,"./main/selsongl.rom"},
    {0x081207C5,"./main/optionl.rom"},
    {0x08122964,"./main/selrolel.rom"},
    {0x081229DF,"./main/cardinfol.rom"},
    {0x08122F97,"./main/signl.rom"},
    {0x08123045,"./main/resultl.rom"},
    {0x08123537,"./main/irl.rom"},
    {0x08123575,"./main/rankl.rom"},
    {0x0812364D,"./main/stylel.rom"},
    {0x0816E7E0,"write-avi.so"},
    {0x0816E820,"snd-oss.so"}
};

// Localize the Song Test Menu 
void Localize_MTVMenu(void){
    int num_str_entries;
    int curr_entry;
    void** cur_addr;
    num_str_entries = sizeof(test_menu_english) / sizeof(StrReplaceEntry);
    for(curr_entry=0; curr_entry < num_str_entries; curr_entry++){
        UnprotectPage(test_menu_english[curr_entry].offset);
        cur_addr = (void*)test_menu_english[curr_entry].offset;
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

void Patch_FilesystemPaths(void){
    unsigned int num_entries_in_table = sizeof(linux_path_entries) / sizeof(StrReplaceEntry);
    for(int i=0;i<num_entries_in_table;i++){
        UnprotectPage(linux_path_entries->offset);
        strcpy((char*)linux_path_entries->offset,linux_path_entries->str);
    }
}


