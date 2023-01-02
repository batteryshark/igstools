#ifndef __SONG_MANAGER_H
#define __SONG_MANAGER_H




void GetSongState(void* state);
void SongManager_Init(void* setting);
void SongManager_Reset(void);
void SongManager_Start(void);
void SongManager_Stop(void);
void SongManager_Update(void* msg);
void GetSongResult(void* request, void* response);
#endif
