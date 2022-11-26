#ifndef A27__H
#define A27__H


#define PCCARD_PATH "/dev/pccard0"
#define PCCARD_READ_OK 0xF1
#define A27_WRITE_HEADER_SIZE 36
#define A27_READ_HEADER_SIZE 132
#pragma pack(1)

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

struct trackball_data{
	unsigned short player_index; 
    unsigned short align;
	unsigned short vx;
	unsigned short vy;
	unsigned char direction;
	unsigned char press;
	unsigned char pulse;
    unsigned char align;
};

#define A27IO_ISSET(v,b) ((v >> b) & 0x01)

enum A27_IOLayout{
    INP_P1_DRUM_4,
    INP_P1_DRUM_1,
    INP_P1_DRUM_2,
    INP_HIDDEN_1,
    INP_HIDDEN_2,
    INP_P1_DRUM_5=5,
    INP_P1_DRUM_6,
    INP_P1_DRUM_3,
    INP_P2_DRUM_1,
    INP_P2_DRUM_2,
    INP_P2_DRUM_3,
    INP_P2_DRUM_4,
    INP_P2_DRUM_5,
    INP_P2_DRUM_6,
    INP_HIDDEN_3=26,
    INP_HIDDEN_4=27,    
    INP_SW_SERVICE=30,
    INP_SW_TEST
};

enum PM_Noteskin{
    NOTESKIN_NORMAL,
    NOTESKIN_ANIMAL,
    NOTESKIN_OCTO,
    NOTESKIN_CHARA,
    NOTESKIN_CAR
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

struct song_packet_3{
    unsigned short cmd;
    unsigned char state;
    unsigned char stage_num;
    unsigned char game_mode;
    unsigned char key_record_mode;
    unsigned char p1_enable;
    unsigned char p2_enable;
    unsigned char p1_autoplay;
    unsigned char p2_autoplay;
    unsigned char p1_songversion;
    unsigned char p2_songversion;
    unsigned short p1_songid;
    unsigned short p2_songid;
    unsigned char p1_speed;
    unsigned char p1_cloak;
    unsigned char p1_noteskin;
    unsigned char align_1;
    unsigned char p2_speed;
    unsigned char p2_cloak;
    unsigned char p2_noteskin;
    unsigned char align_2;
    unsigned short judge_great; 
    unsigned short judge_cool;
    unsigned short judge_nice;
    unsigned short judge_poor;
    unsigned short align_3;
    unsigned char p1_rating;
    unsigned char p2_rating;
    float level_rate_p1[6];
    float level_rate_p2[6];
    unsigned char idk_1[8];
    unsigned char idk_p1_1;
    unsigned char idk_p1_2;
    unsigned char is_non_challengemode;
    unsigned char song_mode;
};


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

#endif  /* A27__H */