#ifndef __SONG_MANAGER_H
#define __SONG_MANAGER_H


unsigned int SongManager_Init(void* setting_data, void* response_buffer);
unsigned int SongManager_Reset(void* response_buffer);
unsigned int SongManager_Start(void* response_buffer);
unsigned int SongManager_Update(void* response_buffer);
void SongManager_Stop(void);
unsigned int SongManager_GetResult(void* response_buffer);

#endif
