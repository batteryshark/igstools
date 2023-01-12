#ifndef __A27_EMULATOR_H
#define __A27_EMULATOR_H


// System Configuration Constants
#define PCCARD_PATH "/dev/pccard0"
#define PCCARD_READ_OK 0xF1
#define PCCARD_FAKE_FD 0xA271337


#define IN_ROM_NAME "S106US"
#define EXT_ROM_NAME "E108US"
#define PCI_CARD_VERSION 100
#define A27_IO_MAX_CHANNELS 6

#include "a27.h"

typedef struct _TEST_READWRITE_DATA{
	unsigned short enabled_flag;
	unsigned short counter;
	char text[256];
}TestReadWriteData,*PTestReadwriteData;

typedef struct _A27_STATE{
    A27ReadMessage  msg;
    unsigned short coin_counter;
    TestReadWriteData test_text[21];
    TrackballData trackball[2];
}A27State,*PA27State;

enum PM_Region{
    REGION_TAIWAN,
    REGION_CHINA,
    REGION_HONGKONG,
    REGION_INTERNATIONAL,
    REGION_AMERICA,
    REGION_EN,
    REGION_EU,
    REGION_KOREA,
    REGION_THAILAND,
    REGION_INTL,
    REGION_RUSSIA
};

enum PM_Noteskin{
    NOTESKIN_NORMAL,
    NOTESKIN_ANIMAL,
    NOTESKIN_OCTO,
    NOTESKIN_CHARA,
    NOTESKIN_CAR
};

enum PM_Difficulty_Mode{
    GAME_MODE_ROOKIE,
    GAME_MODE_EASY,
    GAME_MODE_NORMAL,
    GAME_MODE_HARD,
    GAME_MODE_SPECIAL,
    GAME_MODE_CHALLENGE,
    GAME_MODE_PERCUSSION_MASTER,
    GAME_MODE_NONE,
    GAME_MODE_BATTLE
};

enum A27_Program{
    A27_MODE_HEADER_UPDATE,
	A27_MODE_READWRITE_TEST = 1,
	A27_MODE_KEY_TEST = 2,
	A27_MODE_LIGHT_TEST = 3,
	A27_MODE_COUNTER_TEST = 4,
	A27_MODE_TRACKBALL = 5,
    A27_MODE_6,
	A27_MODE_SCREEN_COIN = 11,
	A27_MODE_SCREEN_MODE = 13,
	A27_MODE_SCREEN_SONG = 14,
	A27_MODE_SONG = 15,
	A27_MODE_SCREEN_RANKING = 20,
	A27_MODE_CCD_TEST = 25,
	A27_MODE_RESET = 27
};




// Internal Firmware Bindings
void A27_Program_1(PA27State a27_state);
void A27_Program_2(PA27State a27_state);
void A27_Program_3(PA27WriteMessage req, PA27ReadMessage res);
void A27_Program_4(PA27State a27_state);
void A27_Program_5(PA27State a27_state);
void A27_Program_11(PA27WriteMessage req, PA27ReadMessage res);
void A27_Program_13(PA27WriteMessage req, PA27ReadMessage res);
void A27_Program_14(PA27WriteMessage req, PA27ReadMessage res);
void A27_Program_15(PA27WriteMessage req, PA27ReadMessage res);
void A27_Program_20(PA27WriteMessage req, PA27ReadMessage res);
void A27_Program_25(PA27State a27_state);

// API for Hook
void A27Emu_Read(PA27ReadMessage msg);
void A27Emu_Write(PA27WriteMessage msg);
void A27Emu_Reset(void);
#endif
