// Utility Functions for A27 Communication

#include "a27.h"

// Derive the challenge response codes for read packets.
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

// Calculate the checksum values on read packets.
void A27SetReadChecksum(PA27ReadHeader msg){
    unsigned int cval = (msg->a27_has_message & 0xFF) + \
        (msg->inet_password_data & 0xFF) + \
        msg->is_light_io_reset + \
        (msg->asic_errnum & 0xFF) + \
        msg->asic_iserror + \
        msg->coin_inserted + \
        (msg->data_size & 0xFF) + \
        (msg->system_mode & 0xFF);
    msg->checksum_1 = (cval & 0xFF);
    msg->checksum_2 = (cval & 0xFF);
}

// Calculate the checksum values on write packets.
void A27SetWriteChecksum(PA27WriteHeader msg){
    unsigned int cval = (msg->key_sensitivity_value & 0xFF)+ \
    (msg->light_disable  & 0xFF)+ \
    (msg->key_input  & 0xFF) + \
    (msg->data_size  & 0xFF) + \
    (msg->system_mode  & 0xFF);        
    msg->checksum_1 = (cval & 0xFF);
    msg->checksum_2 = (cval & 0xFF);
}
