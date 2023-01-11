// IGS PCCARD ASIC Driver for PercussionMaster
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dlfcn.h>

#include "a27.h"
#include "../io/keyio.h"
#include "emulator.h"

// Current Emulator State
A27State a27_state;


void A27Emu_Process(PA27WriteMessage msg){
    // We should update the coin state if there is an update at this point.
    a27_state.msg.header.coin_inserted = KeyIO_GetCoinState();
    if(a27_state.msg.header.coin_inserted){
    if(a27_state.coin_counter == 0xFFFF){a27_state.coin_counter = 0;}
        a27_state.coin_counter++;
    }
    // We can update the switch state here, too.
    a27_state.msg.header.num_io_channels = 1;    
    a27_state.msg.header.button_io[0] = msg->header.key_input;
        
    // The rest depends on the program being executed...
    switch(msg->header.system_mode){
        case A27_MODE_READWRITE_TEST:            
            A27_Program_1(&a27_state);
            break;
        case A27_MODE_KEY_TEST:
            A27_Program_2(&a27_state);
            break;
        case A27_MODE_LIGHT_TEST:
            A27_Program_3(msg,&a27_state.msg);
            break;    
        case A27_MODE_COUNTER_TEST:
            A27_Program_4(&a27_state);
            break;  
        case A27_MODE_TRACKBALL:
            A27_Program_5(&a27_state);
            break;
        case A27_MODE_SCREEN_COIN:
            A27_Program_11(msg,&a27_state.msg);
            break;
        case A27_MODE_SCREEN_MODE:
            A27_Program_13(msg,&a27_state.msg);
            break;
        case A27_MODE_SCREEN_SONG:
            A27_Program_14(msg,&a27_state.msg);
            break;
        case A27_MODE_SCREEN_RANKING:
            A27_Program_20(msg,&a27_state.msg);
            break;  
        case A27_MODE_SONG:
            A27_Program_15(msg,&a27_state.msg);
            break;
        case A27_MODE_CCD_TEST:
            A27_Program_25(&a27_state);
            break;
        default:// Anything unhandled we don't care about.
            a27_state.msg.header.system_mode = msg->header.system_mode;
            a27_state.msg.header.data_size = msg->header.data_size;
            break;
    }
}

// --- Exports ---
void A27Emu_Reset(void){
    memset(&a27_state,0,sizeof(A27State));
    printf("[A27Emu::A27_Reset]\n");

    // Technically, the initial reads from the card start the number of IO channels at 6,
    // But once the game starts responding to actual packets, this gets set to 1 or sometimes 3?
    a27_state.msg.header.num_io_channels = 1;
    a27_state.msg.header.protection_value = rand() & 0xFF;
    a27_state.msg.header.protection_offset = A27DeriveChallenge(a27_state.msg.header.protection_value);
    a27_state.msg.header.game_region = REGION_AMERICA;
    a27_state.msg.header.align_1 = REGION_AMERICA;
    strcpy(a27_state.msg.header.in_rom_version_name,IN_ROM_NAME);
    strcpy(a27_state.msg.header.ext_rom_version_name,EXT_ROM_NAME);
    a27_state.msg.header.inet_password_data = 0xFFFF;
    a27_state.msg.header.pci_card_version = PCI_CARD_VERSION;

    // Reset the Test Read/Write Buffers
    strcpy(a27_state.test_text[0].text,"Test1");
    a27_state.test_text[1].enabled_flag = 1;
    strcpy(a27_state.test_text[1].text,"Test2");
}

void A27Emu_Write(PA27WriteMessage msg){
    A27Emu_Process(msg);
}

void A27Emu_Read(PA27ReadMessage msg){
    memcpy((unsigned char*)msg,&a27_state.msg.header,sizeof(A27ReadHeader));
    memcpy((unsigned char*)msg+sizeof(A27ReadHeader),&a27_state.msg.data,a27_state.msg.header.data_size);
    A27SetReadChecksum((PA27ReadHeader)msg); 
}
