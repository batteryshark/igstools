#ifndef __KEYIO_H
#define __KEYIO_H

unsigned int KeyIO_GetSwitches(void);
unsigned int KeyIO_GetCoinState(void);
void KeyIO_InjectWrite(unsigned char* buf);
void KeyIO_InjectRead(unsigned char* buf);
void KeyIO_Init(void);
#endif
