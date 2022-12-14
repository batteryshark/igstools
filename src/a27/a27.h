#ifndef __A27_H
#define __A27_H


#define PCCARD_PATH "/dev/pccard0"
#define PCCARD_READ_OK 0xF1




// Enums

enum A27_Region{
    REGION_TAIWAN,
    REGION_CHINA = 1,
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



enum A27_LightTest{
    LIGHT_2P_3 = 4,
    LIGHT_YB79,
    LIGHT_YB810,
    LIGHT_2P_1,
    LIGHT_2P_2,
    LIGHT_1P_1 = 12,
    LIGHT_1P_2,
    LIGHT_1P_3,
    LIGHT_YA1,
    LIGHT_YA2,
    LIGHT_YA3,
    LIGHT_YA4,
};

enum PM_Noteskin{
    NOTESKIN_NORMAL,
    NOTESKIN_ANIMAL,
    NOTESKIN_OCTO,
    NOTESKIN_CHARA,
    NOTESKIN_CAR
};

enum A27_Mode{
	A27_MODE_READWRITE_TEST = 1,
	A27_MODE_KEY_TEST = 2,
	A27_MODE_LIGHT_TEST = 3,
	A27_MODE_COUNTER_TEST = 4,
	A27_MODE_TRACKBALL = 5,
	A27_MODE_COIN = 11,
	A27_MODE_SELECT_MODE = 13,
	A27_MODE_SELECT_SONG = 14,
	A27_MODE_SONG = 15,
	A27_MODE_RANKING = 20,
	A27_MODE_CCD_TEST = 25,
	A27_MODE_RESET = 27
};

enum SONG_SUBCMD{
 A27_SONGMODE_PLAYBACK_HEADER,
 A27_SONGMODE_PLAYBACK_BODY,
 A27_SONGMODE_2,
 A27_SONGMODE_MAINGAME_SETTING,
 A27_SONGMODE_MAINGAME_WAITSTART,
 A27_SONGMODE_MAINGAME_START,
 A27_SONGMODE_MAINGAME_PROCESS,
 A27_SONGMODE_7,
 A27_SONGMODE_8,
 A27_SONGMODE_RESULT,
 A27_SONGMODE_10,
 A27_SONGMODE_RESULTDATA_SET,
 A27_SONGMODE_RESULTDATA_COMPLETE,
};

enum game_mode{
    GAME_MODE_TRAINING,
    GAME_MODE_ELEMENTARY,
    GAME_MODE_INTERMEDIATE,
    GAME_MODE_ADVANCED,
    GAME_MODE_SUPER_ADVANCED,
    GAME_MODE_CHALLENGE,
    GAME_MODE_DRUM_KING,
    GAME_MODE_END,
    GAME_MODE_BATTLE
};

enum song_mode{
    SONG_NORMAL,
    SONG_DEMO,
    SONG_OPENING,
    SONG_STAFF,
    SONG_HOW_TO_PLAY
};

// Packet Structures
typedef struct _TRACKBALL_DATA{
	unsigned short player_index; 
    unsigned short align;
	unsigned short vx;
	unsigned short vy;
	unsigned char direction;
	unsigned char press;
	unsigned char pulse;
    unsigned char align2;
}TRACKBALL_DATA;




typedef struct _TEST_READWRITE_DATA{
	unsigned short enabled_flag;
	unsigned short counter;
	char text[256];
}TEST_READWRITE_DATA;

#define A27_READ_HEADER_SIZE 132
struct A27_Read_Message{
    unsigned int dwBufferSize; 
    unsigned int system_mode;     
    unsigned char coin_inserted; 
    unsigned char asic_iserror;  
    unsigned short asic_errnum;  
    unsigned int button_io[6];
    unsigned short num_io_channels;
    unsigned char protection_value;   
    unsigned char protection_offset;  
    unsigned short game_region;    
    unsigned short align_1;
    char pch_in_rom_version_name[8];  
    char pch_ext_rom_version_name[8]; 
    unsigned short inet_password_data; 
    unsigned short a27_has_message;    
    unsigned char is_light_io_reset;   
    unsigned char pci_card_version;    
    unsigned char checksum_1;          
    unsigned char checksum_2;          
    unsigned char a27_message[0x40];              
    unsigned char data[0x4000];
};

#define A27_WRITE_HEADER_SIZE 36
struct A27_Write_Message{
	unsigned int dwBufferSize;
	unsigned int system_mode;
	unsigned int key_input;
	TRACKBALL_DATA trackball;
    unsigned char checksum_1;
    unsigned char checksum_2;
	unsigned char bLightDisable;
	unsigned char key_sensitivity_value;
	unsigned char ucLightState[4];
	unsigned char ucLightPattern[4];
    unsigned char data[0x4000];
};


// Util Functions
unsigned char A27DeriveChallenge(unsigned char inval);
void A27SetReadChecksum(struct A27_Read_Message* msg);
void A27SetWriteChecksum(struct A27_Write_Message* msg);

#endif
