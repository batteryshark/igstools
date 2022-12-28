// Helpers for the Song Mode Logic - By Necessity
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include "a27.h"
#include "../utils.h"
#include "song.h"


#define MAX_CURSORS 150
#define CURSOR_MAX_Y 0x1A0
#define CURSOR_BEAT_ZONE_START 0x150
#define CURSOR_BEAT_ZONE_END 0x190
#define CURSOR_MIN_Y 0

#define MAX_SOUND_EVENTS 100
#define MAX_CURSOR_EVENTS 1000
#define MAX_CURSORS 150

typedef struct _SOUNDEVENT{
    short event_beat;
    short event_value;
}SoundEvent,*PSoundEvent;

typedef struct _CURSOREVENT{
    short event_beat;
    unsigned char cursor_lane;
    unsigned char cursor_flags;
}CursorEvent,*PCursorEvent;

static SoundEvent song_sound_events[MAX_SOUND_EVENTS];
static short sound_index[32];

static CursorEvent song_cursor_events[MAX_CURSOR_EVENTS];

static NoteCursor p1_cursors[MAX_CURSORS];
static NoteCursor p2_cursors[MAX_CURSORS];

static pthread_t songtimer_hthread;
static pthread_t song_soundevent_hthread;
static pthread_t song_cursorevent_hthread;
static pthread_t song_scrolling_hthread;
static short current_beat = -1;
static char in_song = 0;
static char p1_enable = 0;
static char p2_enable = 0;
static char p1_velocity = 1;
static char p2_velocity = 1;


static short song_tempo;



void GetSoundIndex(void* sidx){
        memcpy(sidx,&sound_index,64);
}

void ClearSoundIndex(void){
        memset(&sound_index,0,64);
}

void GetCursorState(unsigned char player_index,void* dst){
 if(player_index){
        memcpy(dst,&p2_cursors,sizeof(NoteCursor) * MAX_CURSORS);
 }else{
     memcpy(dst,&p1_cursors,sizeof(NoteCursor) * MAX_CURSORS);
 }
}

void RemoveCursor(PNoteCursor pcursor){
    pcursor->cursor_exflags = 0;
    pcursor->cursor_flags = 0;
    pcursor->cursor_ypos = 0;
    pcursor->cursor_holdflags = 0;
    pcursor->cursor_stretch = 0;
}

void ShowCursor(PNoteCursor pcursors, unsigned char cursor_slot){
    pcursors[cursor_slot].cursor_flags |= 2;
}

void HideCursor(PNoteCursor pcursors, unsigned char cursor_slot){
    pcursors[cursor_slot].cursor_flags &= ~2;
}

unsigned int AddCursor(PNoteCursor pcursors, unsigned char lane_slot, unsigned char visible, unsigned char cursor_exflags, unsigned char cursor_holdflags, short cursor_ypos){
     
    for(int i=0;i<150;i++){
            if(!pcursors[i].cursor_flags){
            pcursors[i].cursor_flags = (lane_slot << 6);
            
            if(visible){
                pcursors[i].cursor_flags |= 2;    
            }
            pcursors[i].cursor_exflags = cursor_exflags;
            pcursors[i].cursor_holdflags = cursor_holdflags;
            pcursors[i].cursor_ypos = cursor_ypos;
     
             return i;   
            }
    }
    return 0;
}

void ScrollCursors(PNoteCursor pcursors, unsigned short y_velocity){
    for(int i = 0; i < MAX_CURSORS; i++){ 
        pcursors[i].cursor_ypos = pcursors[i].cursor_ypos+y_velocity;
 
        if(pcursors[i].cursor_ypos >= CURSOR_MAX_Y){
                RemoveCursor(pcursors+i);
        }
        // TODO: Something about stretch cursors if necessary.
    }
}


short GetCurrentBeat(void){
    return current_beat;
}

static void *song_timer_thread(void* arg){
    // At the cost of a little loss of precision, we're gonna cut the decimal of the tempo out.
    song_tempo = (unsigned int) song_tempo / 100;

    // First - Given Beats Per Minute, calculate how many ms per beat. (quarter note)
    float ms_per_beat = (float)((60 * 1000) / song_tempo);
    // Next - Divide ms per measure by 8 to get the ms per eighth of a beat (32nd note)
    unsigned int ms_per_ebeat = (int)(ms_per_beat / 8);


    // While the song is playing, we'll determine the beat based on current time since song start.
    long long song_start = GetCurrentTimestamp();
    long long elapsed_time = 0;
    while(in_song){        
        // Then, basically we check the current elapsed time divided by number of ms per beat and we should get what beat we're on.
        //current_beat++;
        elapsed_time = GetCurrentTimestamp() - song_start;
        current_beat = (int)(elapsed_time / ms_per_ebeat);
        
    }
}



static void* song_soundevent_thread(void* arg){
    short last_beat = -1;
    short cur_beat = -1;
    while(in_song){
        cur_beat = GetCurrentBeat();
        // We will only process on 8th beat 1+ and on the new beat.
        if(last_beat != cur_beat && cur_beat > 0){
            for(int i = 0; i < MAX_SOUND_EVENTS; i++){
                // If our queued event doesn't have a beat assignment or it's not the current beat, skip.
                if(!song_sound_events[i].event_beat || song_sound_events[i].event_beat != cur_beat){continue;}
                // If this is a sound cue for this beat, we'll load it into the first available slot.
                for(int j=16;j<32;j++){                
                    if(!sound_index[j]){
                        sound_index[j] = song_sound_events[i].event_value;            
                        // We'll nuke the cue's beat assignment to skip a conditional next time.
                        song_sound_events[i].event_beat = 0;
                        break;
                    }
                }
            }            
        }
        last_beat = cur_beat;
    }
}

static void* song_scrolling_thread(void* arg){
       short last_beat = -1;
    short cur_beat = -1;
    unsigned short p1v = 8;
    unsigned short p2v = 8;
    // While the song is playing, we'll determine the beat based on current time since song start.

    while(in_song){
               cur_beat = GetCurrentBeat();
        // We will only process on 8th beat 1+ and on the new beat.
        if(last_beat != cur_beat){
             if(p1_enable ){
                ScrollCursors(p1_cursors,p1v);    
                
            }
            if(p2_enable ){
                ScrollCursors(p2_cursors,p2v);    
               
            }   
            
        }
         
            
                   last_beat = cur_beat;
        }       

        

}

static void* song_cursorevent_thread(void* arg){
    short last_beat = -1;
    short cur_beat = -1;
    while(in_song){
        cur_beat = GetCurrentBeat();
        // We will only process on 8th beat 1+ and on the new beat.
        if(last_beat != cur_beat){
            for(int i = 0; i < MAX_CURSOR_EVENTS; i++){        
                // If our queued event doesn't have a beat assignment or it's not the current beat, skip.
                if(!song_cursor_events[i].event_beat || song_cursor_events[i].event_beat != cur_beat){continue;}
                // Do P1
                if(p1_enable){
                    for(int j = 0; j < MAX_CURSORS;j++){
                        // TODO: Remove This Logic, Removing hardcoded measures for now.
                        if((p1_cursors[j].cursor_exflags & 0x40) > 0){
                            p1_cursors[j].cursor_ypos = 0;
                            break;
                        }
                        if(p1_cursors[j].cursor_flags == 0){
                            p1_cursors[j].cursor_flags = (song_cursor_events[i].cursor_lane << 6);
                            // We're gonna assume it's visible.
                            p1_cursors[j].cursor_flags |= 2;
                            // Eventually, this will have to be pulled apart for holds etc.
                            p1_cursors[j].cursor_exflags = song_cursor_events[i].cursor_flags;
                            p1_cursors[j].cursor_ypos = 0;
                            break;
                        }
                    }
                }
                // Do P2
                if(p2_enable){
                    for(int j = 0; j < MAX_CURSORS;j++){
                        if(p2_cursors[j].cursor_flags == 0){
                            p2_cursors[j].cursor_flags = (song_cursor_events[i].cursor_lane << 6);
                            // We're gonna assume it's visible.
                            p2_cursors[j].cursor_flags |= 2;
                            // Eventually, this will have to be pulled apart for holds etc.
                            p2_cursors[j].cursor_exflags = song_cursor_events[i].cursor_flags;
                            p2_cursors[j].cursor_ypos = 0;
                            break;
                        }
                    }                    
                }
                
                // We'll nuke the beat assignment to skip this check next time.
                song_cursor_events[i].event_beat = 0;
            }
            
            if(p1_enable){
             if(cur_beat % 32 == 0){
                    AddCursor(p1_cursors,0,1,0x40,0,0);
                }   
            }
            if(p2_enable){
             if(cur_beat % 32 == 0){
                    AddCursor(p2_cursors,0,1,0x40,0,0);
                }   
            }
        }
        last_beat = cur_beat;
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


void StopSong(void){
        in_song = 0;
        // TODO: Reset Shit
        current_beat = -1;
        p1_enable = 0;
        p2_enable = 0;
        memset(p1_cursors,0x00,sizeof(NoteCursor)*MAX_CURSORS);
        memset(p2_cursors,0x00,sizeof(NoteCursor)*MAX_CURSORS);
        
}

// Loads an event file which is essentially a replacement for what's on the PCI card's ROM.
void load_event_file(unsigned int song_mode, unsigned int song_id){
    char sef_path[1024] = {0x00};
    const char* sef_template = "event/%d_%d.sef";
    sprintf(sef_path,sef_template,song_mode,song_id);
    FILE* fp = fopen(sef_path,"rb");
    if(!fp){
     printf("Error! Could not Open SEF File: %s\n",sef_path);
     exit(-1);
    }else{
     printf("Loaded SEF File: %s\n",sef_path);   
    }
    
    unsigned int num_sound_events = 0;
    unsigned int num_cursor_events = 0;
    fread(&num_sound_events,4,1,fp);
    fread(&num_cursor_events,4,1,fp);
    for(int i = 0; i < num_sound_events;i++){
        fread(&song_sound_events[i].event_beat,2,1,fp);
        fread(&song_sound_events[i].event_value,2,1,fp);
    }
    for(int i = 0; i < num_cursor_events; i++){
        fread(&song_cursor_events[i].event_beat,2,1,fp);
        fread(&song_cursor_events[i].cursor_lane,1,1,fp);
        fread(&song_cursor_events[i].cursor_flags,1,1,fp);
    }
    
    fclose(fp);
}

void StartSong(unsigned int song_mode, unsigned int p1_song_id, unsigned int p2_song_id){
        StopSong();
        memset(&song_sound_events,0x00,sizeof(SoundEvent)*MAX_SOUND_EVENTS);
        memset(&song_cursor_events,0x00,sizeof(CursorEvent) * MAX_CURSOR_EVENTS);
        unsigned int song_id = (p1_song_id > p2_song_id) ? p1_song_id : p2_song_id;       
        p1_enable = (p1_song_id > 0);
        p2_enable = (p2_song_id > 0);
        
        load_event_file(song_mode,song_id);
        song_tempo = DeriveBPM(song_mode,song_id);
        printf("Song BPM Set to: %d\n",song_tempo);
        in_song = 1;        
        // Start Thread
        pthread_create(&songtimer_hthread, 0, song_timer_thread, 0);
        pthread_create(&song_soundevent_hthread, 0, song_soundevent_thread, 0);
        pthread_create(&song_cursorevent_hthread,0,song_cursorevent_thread,0);
        pthread_create(&song_scrolling_hthread,0,song_scrolling_thread,0);
        
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










/*
- Sound Event Thread : checks the current beat from the last beat and if it's a new beat, we pull any cues from our sound thread and chuck em on.


 So Essentially What we Need Now is like a ... song event thread, it will run parallel to the song timer thread and constantly watch for beat changes.
 On each new eighth beat, it will look for sound events to queue up.
 It will also look up cursor events to queue up - understanding if the song was started with p1 and p2 or one or both to add to the right queue.
 
 
 for judgement, when a note is in the 'beat zone', if you hit it, it will disappear and no longer be judged 
 
  
*/
 
