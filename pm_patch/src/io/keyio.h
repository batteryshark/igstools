#ifndef __KEYIO_H
#define __KEYIO_H

#include "../a27/a27.h"

enum KEYIO_PLAYER_KEY{
 KEYIO_PBLUE,
 KEYIO_PDRUM_L,
 KEYIO_PDRUM_R,
 KEYIO_PRIM_L,
 KEYIO_PRIM_R,
 KEYIO_PRED
};

enum KEYIO_SYSTEM_KEY{
    KEYIO_STEST,
    KEYIO_SSVC,
    KEYIO_DEV_1,
    KEYIO_DEV_2,
    KEYIO_COIN
};

typedef struct _KEYIO_STATE{
    unsigned char p1[6];
    unsigned char p2[6];
    unsigned char system[5];
}KeyIOState,*PKeyIOState;


PKeyIOState KeyIO_GetState(void);
unsigned int KeyIO_GetSwitchState(void);
unsigned int KeyIO_GetCoinState(void);
void KeyIO_InjectWrite(PA27WriteHeader write_header);
void KeyIO_InjectRead(PA27ReadHeader read_header);
void KeyIO_Init(void);
#endif
