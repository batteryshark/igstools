#ifndef __KEYIO_H
#define __KEYIO_H

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
