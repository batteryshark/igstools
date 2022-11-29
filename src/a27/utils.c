#include "a27.h"

static unsigned char m_baDecodeTable[10] = {6, 7, 3, 4, 8, 0, 1, 2, 9, 5};

unsigned char A27DeriveChallenge(unsigned char inval){
    unsigned char target_value = inval % 10;
    int i;
    for(i=0;i<sizeof(m_baDecodeTable);i++){
        if(m_baDecodeTable[i] == target_value){
            return i;
        }
    }
    return 0;
}

void A27SetReadChecksum(struct A27_Read_Message* msg){
    unsigned int cval = (msg->a27_has_message & 0xFF) + \
        (msg->inet_password_data & 0xFF) + \
        msg->is_light_io_reset + \
        (msg->asic_errnum & 0xFF) + \
        msg->asic_iserror + \
        msg->coin_inserted + \
        (msg->dwBufferSize & 0xFF) + \
        (msg->system_mode & 0xFF);
    msg->checksum_1 = (cval & 0xFF);
    msg->checksum_2 = (cval & 0xFF);
}


void A27SetWriteChecksum(struct A27_Write_Message* msg){

    unsigned int cval = (msg->key_sensitivity_value & 0xFF)+ \
    (msg->bLightDisable  & 0xFF)+ \
    (msg->key_input  & 0xFF) + \
    (msg->dwBufferSize  & 0xFF) + \
    (msg->system_mode  & 0xFF);        
    msg->checksum_1 = (cval & 0xFF);
    msg->checksum_2 = (cval & 0xFF);
}
