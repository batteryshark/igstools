#ifndef __A27_LOG_H
#define __A27_LOG_H
#include <stddef.h>
void A27Log_Read(void* buffer);
void A27Log_Write(const void* buffer, size_t count);
void A27Log_PacketId(off_t value);
void A27Log_init(void);
#endif
