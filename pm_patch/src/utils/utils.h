#ifndef __UTILS__H
#define __UTILS__H
// -- Debug
void PrintHex(unsigned char* data, unsigned int len);
// -- System
void QuitProcess(void);
// -- Patching
void UnprotectPage(int addr);
void PatchCall(void* call_address, void* target_address);
void PatchJump(void* jump_address, void* target_address);
// -- Time
long long GetCurrentTimestamp();
// -- Conversion
float Short2Float(short val);
void SleepMS(unsigned int num_ms);
unsigned short DeriveDistanceByTime(long start_time, long current_time, long end_time, long start_position, long end_position);
#endif
