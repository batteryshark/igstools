#ifndef __KEYIO_H
#define __KEYIO_H

typedef struct _drum_state{
    unsigned char rim_l;
    unsigned char drum_l;
    unsigned char drum_r;
    unsigned char rim_r;
    unsigned char blue;
    unsigned char red;
}drum_state;

struct iostate {
    unsigned char hidden_sw[2];
    unsigned char coin;
    unsigned char sw_test;
    unsigned char sw_service;
    drum_state p1;
    drum_state p2;
};

struct iostate* KeyIO_GetState(void);
unsigned int KeyIO_GetSwitches(void);
unsigned int KeyIO_GetSwitchesCurrent(void);
unsigned int KeyIO_GetCoinState(void);
void KeyIO_InjectWrite(unsigned char* buf);
void KeyIO_InjectRead(unsigned char* buf);
void KeyIO_Init(void);
#endif
