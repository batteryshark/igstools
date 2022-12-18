#ifndef __SNDMGR_H
#define __SNDMGR_H
void SoundMgr_Init(void);
void SoundMgr_Shutdown(void);
int SoundMgr_Load(const char* path_to_soundfile, int should_loop);
void SoundMgr_LoadCredit(const char* path_to_soundfile);
void SoundMgr_PlayCredit(float volume);
void SoundMgr_Unload(void* sound);
void SoundMgr_UnloadAll(void);
void SoundMgr_Play(unsigned int slot, float volume);
void SoundMgr_Stop(unsigned int slot);
void SoundMgr_Wait(int channel_index);
void SoundMgr_Update(void);
#endif
