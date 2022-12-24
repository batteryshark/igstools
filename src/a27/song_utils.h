#ifndef __SONGUTILS_H
#define __SONGUTILS_H

#include "song.h"

short GetCurrentBeat(void);
void StartSong(unsigned int song_mode, unsigned int p1_song_id, unsigned int p2_song_id);
void StopSong(void);
void GetSongResult(PSongResultRequest req, PSongResult res);
// SoundIndex Stuff
void ResetSoundIndex(PSongState state);
void AddToSoundIndex(PSongState state, unsigned short value);
void CheckForSoundCues(PSongState state);
#endif
