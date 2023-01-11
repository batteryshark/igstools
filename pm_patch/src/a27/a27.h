// Defines for Handling Communication with the A27
#ifndef __A27_H
#define __A27_H

#define PCCARD_DATA_BUFFER_SIZE 0x4000

#define IO_SET(value,bit) (value |= (1 << bit))
#define IO_UNSET(value,bit) (value &= ~(1 << bit))
#define IO_ISSET(value,bit) ((value >> bit) & 1)
#define IO_ISSSET(value,old,bit) (IO_ISSET(value,bit) && !IO_ISSET(old,bit))
#define IO_ISOSET(value,old,bit) (IO_ISSET(value,bit) || IO_ISSET(old,bit))

enum A27_IOLayout{
    INP_P1_DRUM_L,
    INP_P1_DRUM_R,
    INP_P1_RIM_R,
    INP_P1_BLUE=5,
    INP_P1_RED,
    INP_P1_RIM_L,
    INP_P2_BLUE,
    INP_P2_RED,
    INP_P2_RIM_L,
    INP_P2_DRUM_L,
    INP_P2_DRUM_R,
    INP_P2_RIM_R,
    INP_DEV_1=26,
    INP_DEV_2,      
    INP_SW_SERVICE=30,
    INP_SW_TEST
};

typedef struct _TRACKBALL_DATA{
	unsigned short player_index; 
    unsigned short align;
	unsigned short vx;
	unsigned short vy;
	unsigned char direction;
	unsigned char press;
	unsigned char pulse;
    unsigned char align2;
}TrackballData,*PTrackballData;

typedef struct _A27_READ_HEADER{
    unsigned int data_size; 
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
    char in_rom_version_name[8];  
    char ext_rom_version_name[8]; 
    unsigned short inet_password_data; 
    unsigned short a27_has_message;    
    unsigned char is_light_io_reset;   
    unsigned char pci_card_version;    
    unsigned char checksum_1;          
    unsigned char checksum_2;          
    unsigned char a27_message[0x40];              
}A27ReadHeader,*PA27ReadHeader;

typedef struct _A27_READ_MESSAGE{
    A27ReadHeader header;
    unsigned char data[PCCARD_DATA_BUFFER_SIZE];    
}A27ReadMessage,*PA27ReadMessage;

typedef struct _A27_WRITE_HEADER{
	unsigned int data_size;
	unsigned int system_mode;
	unsigned int key_input;
	TrackballData trackball_data;
    unsigned char checksum_1;
    unsigned char checksum_2;
	unsigned char light_disable;
	unsigned char key_sensitivity_value;
	unsigned char light_state[4];
	unsigned char light_pattern[4];    
}A27WriteHeader,*PA27WriteHeader;

typedef struct _A27_WRITE_MESSAGE{
    A27WriteHeader header;
    unsigned char data[PCCARD_DATA_BUFFER_SIZE];
}A27WriteMessage,*PA27WriteMessage;

// Util Functions
unsigned char A27DeriveChallenge(unsigned char inval);
void A27SetReadChecksum(PA27ReadHeader msg);
void A27SetWriteChecksum(PA27WriteHeader msg);
#endif
