#ifndef __A27_EMU_H
#define __A27_EMU_H

#define IN_ROM_NAME "S106US"
#define EXT_ROM_NAME "E108US"
#define PCI_CARD_VERSION 100
#define A27_IO_MAX_CHANNELS 6

void A27Emu_Write(const void* buf);
void A27Emu_Reset(void);
void A27Emu_Read(void* buf);
#endif
