#ifndef __A27_LOG_H
#define __A27_LOG_H


void A27Log_Read(off_t offset,void* buffer);
void A27Log_Write(off_t offset,const void* buffer, size_t count);
void A27Log_init(void);
#endif
