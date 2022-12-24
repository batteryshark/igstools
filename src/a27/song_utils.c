// Helpers for the Song Mode Logic - By Necessity
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include "a27.h"
#include "../utils.h"
#include "song.h"



static pthread_t songtimer_hthread;
static short current_beat = -1;
static char in_song = 0;
static short song_tempo;


short GetCurrentBeat(void){
    return current_beat;
}

static void *song_timer_thread(void* arg){
    // At the cost of a little loss of precision, we're gonna cut the decimal of the tempo out.
    song_tempo = (unsigned int) song_tempo / 100;

    // First - Given Beats Per Minute, calculate how many ms per measure.
    float ms_per_measure = (float)((60 * 1000) / song_tempo);
    // Next - Divide ms per measure by 8 to get the ms per beat (eighth notes)
    unsigned int ms_per_beat = (int)(ms_per_measure / 8);

    in_song = 1;
    // While the song is playing, we'll determine the beat based on current time since song start.
    long long song_start = GetCurrentTimestamp();
    long long elapsed_time = 0;
    while(in_song){        
        // Then, basically we check the current elapsed time divided by number of ms per beat and we should get what beat we're on.
        //current_beat++;
        elapsed_time = GetCurrentTimestamp() - song_start;
        current_beat = (int)(elapsed_time / ms_per_beat);
        
    }
}

struct game_song_info{
	unsigned short tempo; // this is bpm * 100 basically
	unsigned short song_length; // this is weird, let x be song_length: ((x * 0.125) / (tempo / 100) * 60.0) = song length in seconds what the fuck how what?!?
	unsigned char stars; // difficulty ranking
	unsigned char section_id;
	unsigned char song_version; 
	unsigned char idk; 
	unsigned int  chart_difficulty; // from 0-8 (Rookie,Easy,Normal,Hard,Special,Challenge,Drum Master,None, Battle)
	unsigned short num_notes;
	unsigned short other_num_notes; // this looks like slightly less than num_notes?
	unsigned int idk3; // idk, it's 1 on some.
	unsigned short tempo_2; // idk maybe this is a new part of the structure.
	unsigned char idk_somevals[78];
	unsigned char* tracks[36]; // tracks cover cursors and sound cues it looks like.
	unsigned char track_enabled[36]; // marked as 1 if the above track has stuff in it.
	unsigned int idk_align[2];
};

static void* (*song_prRomSongGet)(char always_0, unsigned short song_list_index, char song_mode) = 0x0807AECC;

unsigned short DeriveBPM(unsigned int song_mode, unsigned int song_id){
    
	static const unsigned short opening_bpm = 0x1D9C;
	static const unsigned short howtoplay_bpm = 0x2EE0;
	static const unsigned short staff_bpm = 0x2EE0;

	struct game_song_info* si;
	switch(song_mode){
	case SONG_MODE_NORMAL:
	case SONG_MODE_DEMO:
		si = (struct game_song_info*)song_prRomSongGet(0,song_id,0);
		return si->tempo;
	case SONG_MODE_OPENING:
		return opening_bpm;
	case SONG_MODE_STAFF:
		return staff_bpm;
	case SONG_MODE_HOWTOPLAY:
		return howtoplay_bpm;
	default:
		printf("Error - Invalid SongMode: %d\n",song_mode);
		return (unsigned short)0;
	}
}

void StartSong(unsigned int song_mode, unsigned int p1_song_id, unsigned int p2_song_id){
        unsigned int song_id = (p1_song_id > p2_song_id) ? p1_song_id : p2_song_id;        
        song_tempo = DeriveBPM(song_mode,song_id);
        printf("Song BPM Set to: %d\n",song_tempo);
        // Start Thread
        pthread_create(&songtimer_hthread, 0, song_timer_thread, 0);
}

void StopSong(void){
        in_song = 0;
        // TODO: Reset Shit
        current_beat = -1;
}


void GetSongResult(PSongResultRequest req, PSongResult res){
     
    
    res->cmd = A27_SONGMODE_RESULT;
    res->p1_num_notes = 123;
    res->p2_num_notes = 456;

    res->p1_great = 1;
    res->p1_cool = 2;
    res->p1_nice = 3;
    res->p1_poor = 4;
    res->p1_lost = 5;
    res->song_clear = 1;
    res->enable_bonus_stage = 1;

    res->p2_great = 1;
    res->p2_cool = 2;
    res->p2_nice = 3;
    res->p2_poor = 4;
    res->p2_lost = 5;
    res->p1_max_combo = 6;
    res->p2_max_combo = 6;
    res->p1_fever_beat = 7;
    res->p2_fever_beat = 7;

    
    res->p1_score = 61;
    res->p2_score = 60;
    res->p1_grade = 0; 
    res->p2_grade = 1; 

    // TODO: Calculate Grade
    
    // Calculating Message 
    if(res->p1_num_notes == res->p1_max_combo){
        res->p1_message = SongResulMessage_BRAVO;
        if(res->p1_max_combo == res->p1_cool){
            res->p1_message = SongResulMessage_PERFECT;   
        }
    }

    if(res->p2_num_notes == res->p2_max_combo){
        res->p2_message = SongResulMessage_BRAVO;
        if(res->p2_max_combo == res->p1_cool){
            res->p2_message = SongResulMessage_PERFECT;   
        }
    }
}

// Sound Index Queue Processing
void ResetSoundIndex(PSongState state){
    memset(&state->sound_index,0x00,sizeof(state->sound_index));    
}

// Add sound cue to the first available index.
void AddToSoundIndex(PSongState state, unsigned short value){
    for(int i=0;i<32;i++){
        if(!state->sound_index[i]){
            state->sound_index[i] = value;
            return;
        }
    }
}

// Based on current note, check for any predefined sound cues (like the BGM).
typedef struct _SoundCue{
    short cue_beat;
    short sound_value;
}SoundCue,*PSoundCue;

static SoundCue testsc[1] = {
    //{16,7}
    {10,1}
    
};

static unsigned int num_sound_cues = 1;

void CheckForSoundCues(PSongState state){
    // We won't ever check for cues until the first beat.
    if(current_beat < 1){return;}
    for(int i = 0; i < num_sound_cues; i++){
        if(testsc[i].cue_beat > 0 && testsc[i].cue_beat <= current_beat){
            AddToSoundIndex(state,testsc[i].sound_value);
            testsc[i].cue_beat = 0;
        }
    }
}

