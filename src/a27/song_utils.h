#ifndef __SONGUTILS_H
#define __SONGUTILS_H

void ResetSoundEvents(void* state);
void AddToSoundEvents(void* state, unsigned short event_value);
void SetKeySound(void*state, unsigned char slot, unsigned short value);
void ClearKeySounds(void* state);
unsigned char CalculateVelocity(float tempo, unsigned int speed_mod);
#endif
