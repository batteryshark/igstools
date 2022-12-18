#ifndef __UTILS__H
#define __UTILS__H


void PrintHex(unsigned char* data, unsigned int len);
void Shutdown(void);
void UnprotectPage(int addr);
void PatchCall(void* call_address, void* target_address);
void PatchJump(void* jump_address, void* target_address);
#endif
