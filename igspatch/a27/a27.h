#pragma once


#define PCCARD_PATH "/dev/pccard0"
#define PCCARD_READ_OK 0xF1
#define A27_WRITE_HEADER_SIZE 36
#define A27_READ_HEADER_SIZE 132


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

enum A27_Mode{
	A27_MODE_TEST = 1,
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

#pragma pack(1)
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

struct A27_Write_Message{
	unsigned int dwBufferSize;
	unsigned int system_mode;
	unsigned int key_input_flag;
	unsigned short trackball_data[7];
	unsigned char bLightDisable;
	unsigned char key_sensitivity_value;
	unsigned char ucLightState[4];
	unsigned char ucLightPattern[4];
    unsigned char data[0x4000];
};