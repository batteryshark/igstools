#ifndef __SONGUTILS_H
#define __SONGUTILS_H

#include "song.h"

short GetCurrentBeat(void);
void StartSong(unsigned int song_mode, unsigned int p1_song_id, unsigned int p2_song_id);
void StopSong(void);
void GetSongResult(PSongResultRequest req, PSongResult res);
// SoundIndex Stuff

void AddToSoundIndex(PSongState state, unsigned short value);
void CheckForSoundCues(PSongState state);
unsigned int AddCursor(PNoteCursor pcursors, unsigned char lane_slot, unsigned char visible, unsigned char cursor_exflags, unsigned char cursor_holdflags, short cursor_ypos);
void RemoveCursor(PNoteCursor pcursors,unsigned char cursor_slot);
void ShowCursor(PNoteCursor pcursors, unsigned char cursor_slot);
void HideCursor(PNoteCursor pcursors, unsigned char cursor_slot);
void ScrollCursors(PNoteCursor pcursors, unsigned short y_velocity);
void GetCursorState(unsigned char player_index,void* dst);
void GetSoundIndex(void* sidx);
void ClearSoundIndex(void);
#endif
