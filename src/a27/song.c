// A27 PercussionMaster SongMode Process Module
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>

#include "../keyio.h"
#include "../utils.h"
#include "a27.h"
#include "song.h"



static struct maingame_setting sset = {0x00};
static unsigned char playback_header[148] = {0x00};
static unsigned char playback_body[4004] = {0x00};
static int song_playing = 0;
static pthread_t bgm_hthread;

static unsigned int* internal_soundtable = NULL;


enum JUDGEPROC{
    ANI_JUDGE_GREAT=1,
    ANI_JUDGE_COOL,
    ANI_JUDGE_NICE,
    ANI_JUDGE_POOR,
    ANI_JUDGE_LOST, // Battle Mode
    ANI_JUDGE_BRAVO, // Battle Mode
    ANI_JUDGE_IDK1,
    ANI_JUDGE_IDK2,
    ANI_JUDGE_IDK3
};

#pragma pack(1)
typedef struct _CURSOR_STATE{
    unsigned int cursor_type;
    short cursor_location;
    short cursor_idk;
}NOTECURSOR;

enum SongEffectorAnimation{
 ANI_BLUE,
 ANI_DRUM_L,
 ANI_DRUM_R,
 ANI_RIM_L,
 ANI_RIM_R,
 ANI_RED,
 ANI_PLACEHOLDER_1,
 ANI_PLACEHOLDER_2
};

enum KEYSOUND_INDEX{
 KEYSOUND_BLUE = 1,
 KEYSOUND_RED,
 KEYSOUND_DRUM,
 KEYSOUND_RIM,
 KEYSOUND_2DRUM,
 KEYSOUND_2RIM
};


static struct _SONGSTATE{
    unsigned short cmd; // Always 6
    unsigned short state;
    unsigned short p1_note_counter;
    unsigned short p2_note_counter;
    unsigned short sound_index[32];
    NOTECURSOR p1_cursor[150];
    NOTECURSOR p2_cursor[150];
    unsigned short p1_combo; //0x9a8
    unsigned short p2_combo; // 0x9aa
    unsigned char p2_fireworks;
    unsigned char p2_fireworks_2;
    unsigned char p1_fireworks;
    unsigned char p1_fireworks_2;    
    unsigned char p1_playing; // 0x9b0
    unsigned char p2_playing; // 0x9b1
    unsigned char idk_maybepadding[2];
    unsigned char p1_judge_animation[8];
    unsigned char p2_judge_animation[8];
    unsigned char p1_lane_animation[8];
    unsigned char p2_lane_animation[8];
    unsigned char p1_note_hit_animation[8];
    unsigned char p2_note_hit_animation[8];
    unsigned int p1_score; 
    unsigned int p2_score; 
    unsigned int p1_score_2; 
    unsigned int p2_score_2; 
    unsigned int idk_maybepadding2; 
    unsigned short p1_lifebar;
    unsigned short p2_lifebar;
    unsigned short lifebar_align[2];
    
}SongState;



static short current_note;

static unsigned char note_tick_max = 6;
static unsigned char note_tick = 0;

typedef struct _SOUNDINDEX_EVENT{
    unsigned short note_index;
    unsigned char  sound_index;
    unsigned short sound_value;
}SIDXEVT,*PSIDXEVT;

typedef struct _CURSOR_EVENT{
    unsigned short note_index;
    unsigned char cursor_slot;
    unsigned int cursor_type;    
    short cursor_idk;
}NCEVT,*PNCEVT;

static void* soundindex_event_table = NULL;
static unsigned int soundindex_event_table_size = 0;
static void* cursor_event_table = NULL;
static unsigned int cursor_event_table_size = 0;



void Song_UploadPlaybackHeader(const unsigned char* in_data, struct A27_Read_Message* msg){
    memcpy(playback_header,in_data,sizeof(playback_header));
    // Do Nothing for Now
    // TODO: Upload Logic
    // This Effectively means OK?
    msg->dwBufferSize = 4;
    msg->data[0] = 2;
}

void Song_UploadPlaybackBody(const unsigned char* in_data, struct A27_Read_Message* msg){
    memcpy(playback_body,in_data,sizeof(playback_body));
    // Do Nothing for Now
    // TODO: Upload Logic
    // This Effectively means OK?
    msg->dwBufferSize = 4;
    msg->data[0] = 2;
}

enum SefEventTypes{
 SIDXEvent,
 CursorEvent
};

typedef struct _eventheader{
 unsigned int bpm;
 unsigned int num_sidx_events;
 unsigned int num_cursor_events;
}sefheader,*psefheader;

typedef struct _event_entry{
    unsigned char event_size;
    unsigned char event_type;
    short event_note;
    unsigned char event_data[1];
}sefentry,*psefentry;

typedef struct _soundindex_event_entry{
    unsigned char sidx_slot;
    unsigned short sidx_value;
}sidxentry,*psidxentry;

static SIDXEVT* current_sidx;
static unsigned int current_sidx_length;
static unsigned int num_cursor_events = 0;

// Loads an event file which is essentially a replacement for what's on the PCI card's ROM.
void load_event_file(void){
    const char* sef_template = "event/%d_%d_%d.sef";
    char sef_path[1024] = {0x00};
    unsigned int songid = sset.p1_songid;
    unsigned int songversion = sset.p1_songversion;
    if(!songid){
        songid = sset.p2_songid;    
    }
    if(!songversion){
        songversion = sset.p2_songversion;    
    }
    if(!songid || !songversion){
        printf("Error! SongID or SongVersion Cannot be 0!\n");
        exit(-1);
    }
    sprintf(sef_path,sef_template,sset.song_mode,songid,songversion);
    FILE* fp = fopen(sef_path,"rb");
    if(!fp){
     printf("Error! Could not Open SEF File: %s\n",sef_path);
     exit(-1);
    }else{
     printf("Loaded SEF File: %s\n",sef_path);   
    }
    
    fseek(fp,0,SEEK_END);
    unsigned int fsz = ftell(fp);
    rewind(fp);
    unsigned char* event_buffer = (unsigned char*)malloc(fsz);
    fread(event_buffer,fsz,1,fp);
    fclose(fp);
    psefheader hdr = (psefheader)event_buffer;
    unsigned int num_sidx_events = hdr->num_sidx_events;
    soundindex_event_table_size = num_sidx_events * sizeof(SIDXEVT);
    
    num_cursor_events = hdr->num_cursor_events;
    cursor_event_table_size = num_cursor_events * sizeof(NCEVT);
    printf("Event File Info: BPM: %d Num SoundIndex Events: %d Num Cursor Events: %d\n",hdr->bpm,num_sidx_events,num_cursor_events);
    // Now we'll initialize our buffers for the song (and clear them if they havent been already).
    if(soundindex_event_table != NULL){
        free(soundindex_event_table);
    }
    soundindex_event_table = malloc(soundindex_event_table_size);
    if(cursor_event_table != NULL){
     free(cursor_event_table);   
    }
    cursor_event_table = malloc(cursor_event_table_size);
    
    // Now we'll load the events into our tables.
    unsigned int cursor_table_offset = 0;
    unsigned int sidx_table_offset = 0;
    unsigned int offset = sizeof(sefheader);
    while(offset < fsz){
        psefentry ce = (psefentry)(event_buffer+offset);
        PSIDXEVT sidxe;
        PNCEVT nce;
        switch(ce->event_type){
            case SIDXEvent:
                sidxe = (PSIDXEVT)(soundindex_event_table+sidx_table_offset);
                memcpy(sidxe,&ce->event_note,sizeof(SIDXEVT));
                sidx_table_offset+=sizeof(SIDXEVT);
                break;
            case CursorEvent:
                nce = (PNCEVT)(cursor_event_table+cursor_table_offset);
                memcpy(nce,&ce->event_note,sizeof(NCEVT));
                cursor_table_offset+=sizeof(NCEVT);
                break;
            default:
                printf("Error: Unsupported SEF Event Type: %d\n",ce->event_type);
                exit(-1);
            
        }        
        offset += ce->event_size;
    }
    
    current_sidx_length = soundindex_event_table_size / sizeof(SIDXEVT);
    
    
}

void print_song_setting(void){
    printf("--- New Song Setting ---\n");
    printf("State: %d\n",sset.state);
    printf("Stage: %d GameMode: %d KeyRecord:%d\n",sset.stage_num,sset.game_mode,sset.key_record_mode);
    printf("P1 Enable: %d Version: %d SongID: %d\n",sset.p1_enable,sset.p1_songversion,sset.p1_songid);
    printf("P1 Speed: %d Cloak: %d Noteskin: %d Auto: %d\n",sset.p1_speed,sset.p1_cloak,sset.p1_noteskin,sset.p1_autoplay);
    printf("P2 Enable: %d Version: %d SongID: %d\n",sset.p2_enable,sset.p2_songversion,sset.p2_songid);
    printf("P2 Speed: %d Cloak: %d Noteskin: %d Auto: %d\n",sset.p2_speed,sset.p2_cloak,sset.p2_noteskin,sset.p2_autoplay);
    printf("Scoring: G: %d C: %d N: %d P: %d\n",sset.judge_great,sset.judge_cool,sset.judge_nice,sset.judge_poor);
    printf("P1 Rating: %d P2 Rating: %d\n",sset.p1_rating,sset.p2_rating);
    printf("LevelRate P1: Fever %.2f Great %.2f Cool %.2f Nice %.2f Poor %.2f Lost %.2f\n",sset.level_rate_p1[0],sset.level_rate_p1[1],sset.level_rate_p1[2],sset.level_rate_p1[3],sset.level_rate_p1[4],sset.level_rate_p1[5]);
    printf("LevelRate P2: Fever %.2f Great %.2f Cool %.2f Nice %.2f Poor %.2f Lost %.2f\n",sset.level_rate_p2[0],sset.level_rate_p2[1],sset.level_rate_p2[2],sset.level_rate_p2[3],sset.level_rate_p2[4],sset.level_rate_p2[5]);
    printf("IDK 1 [Probably Padding]: ");
    PrintHex(sset.idk_1,8);
    printf("IDK P1: %d P2: %d\n",sset.idk_p1_1,sset.idk_p1_2);
    printf("Is Non Challenge Mode: %d\n",sset.is_non_challengemode);
    printf("Song Mode: %d\n",sset.song_mode);
    printf("--- End Song Setting ---\n");
}



unsigned int last_sw;


// EXPERIMENTAL DIRECT SOUND CALLS -- PARDON OUR DUST!
static unsigned short* pbgm_index_value = (unsigned short*)0x0843E6E0;

static void* m_prSongVolume = (void*)0x083EB85C;
static int* soundslots = (int*)0x083E0F20;
	
static void (*SoundPlay)(unsigned int a1, int volume) = (void (*)(unsigned int, int))0x08065524;


void SetSidx(unsigned short song_mode, unsigned short p1_songid, unsigned short p2_songid){
    
    /*
    switch(song_mode){
        case 2:   
        current_sidx = opening_sidx;
        current_sidx_length = sizeof(opening_sidx) / sizeof(SIDXEVT);
        return;       
        case 4:
        current_sidx = howtoplay_sidx;
        current_sidx_length = sizeof(howtoplay_sidx) / sizeof(SIDXEVT);
        return;                   
        
        default:
            break;
    }  
    */
}

/*
static int* song_sound_table = (int*)0x0843C69C;
int GetSoundSlot(unsigned char index){
    return song_sound_table[index];
}

int PlaySoundAtIndex(unsigned char index, unsigned short value){
    
int volume = 0;
if(index == 16){
	volume = *(unsigned short*)m_prSongVolume+1;
	
		SoundPlay(song_sound_table[value],volume)
; // TODO REMOVE THIS
    return 1;
}else if(index > 15){
	volume = *(unsigned short*)m_prSongVolume+2;
}else{
	volume = *(unsigned short*)m_prSongVolume;	
}


// We won't play the sound if it's not loaded yet.
//if(!soundslots[6 * value]){return 0;}

// song_sound_table[value]
   // printf("Playing Sound %d, %d Vol\n",song_sound_table[value],volume);
	//SoundPlay(song_sound_table[value],volume)
;
    return 1;
}
*/



void Update_Soundindex(){
    
    SIDXEVT* sdx = (SIDXEVT*)soundindex_event_table; 
    // Reset sound_index after keysound slots.
    for(int i=16;i<32;i++){
        SongState.sound_index[i] = 0;
    }
    
    for(int i = 0; i < current_sidx_length;i++){
        if(sdx[i].note_index < 1){continue;};
        if(sdx[i].note_index == current_note){
            SongState.sound_index[sdx[i].sound_index] = sdx[i].sound_value;
           // int res = PlaySoundAtIndex(sdx[i].sound_index,sdx[i].sound_value);
            //if(!res){continue;}
            // We're effectively nuking this item to not be played again.
            sdx[i].note_index = 0;
        }
    }
}

struct iostate last_ks_state;



static void *bgm_thread(void *arg){

   //void* snd = SoundMgr_Load("/peng/linux/band1/song/chichi/1_Conga08.wav");
   //void* bgm = SoundMgr_Load("/peng/linux/band1/song/chichi/BG.wav");
   //SoundMgr_Play(bgm,0);
  
    struct iostate* io_state;
    while(song_playing){        
        Update_Soundindex();
        io_state = KeyIO_GetState();
        
        
    if(io_state->p1.blue && !last_ks_state.p1.blue){
       //SoundMgr_Play(snd,0);
        //PlaySoundAtIndex(0,KEYSOUND_BLUE);
       
    }
    memcpy(&last_ks_state,io_state,sizeof(struct iostate));
        
    }
   
}

static unsigned int cursor_delay = 0;
static unsigned int cursor_delay_max = 0;

void Song_MainGameSetting(const unsigned char* in_data, struct A27_Read_Message* msg){
    memcpy(&sset,in_data,sizeof(sset));
    print_song_setting();

    memset(&SongState,0,sizeof(SongState));
    SongState.cmd = A27_SONGMODE_MAINGAME_SETTING;

    memcpy(msg->data,&SongState,sizeof(SongState));
    msg->dwBufferSize = sizeof(SongState);
    // At this point, we know enough about the song to get the event file...
    load_event_file();
    cursor_delay = 0;

}

void Song_MainGameWaitStart(const unsigned char* in_data, struct A27_Read_Message* msg){
    msg->dwBufferSize = sizeof(SongState);
    note_tick = 0;
    current_note = 0;
    SongState.cmd = A27_SONGMODE_MAINGAME_WAITSTART;
    SongState.p1_note_counter = current_note;
    SongState.p2_note_counter = current_note;

    if(sset.p1_enable){
            SongState.p1_playing = 1;
    }
    if(sset.p2_enable){
            SongState.p2_playing = 1;
    }    
    
    SongState.p1_lifebar = 10; // I don't know what this is for.
    SongState.p2_lifebar = 10;
    memset(msg->data,0,sizeof(SongState));
    *(unsigned short*)msg->data = A27_SONGMODE_MAINGAME_WAITSTART;

}

void Song_MainGameStart(const unsigned char* in_data, struct A27_Read_Message* msg){
    msg->dwBufferSize = sizeof(SongState);
    current_note = -1;
    if(SongState.p1_playing){
        SongState.p1_note_counter = current_note;
    }
    if(SongState.p2_playing){
        SongState.p2_note_counter = current_note;
    }
	SongState.cmd = A27_SONGMODE_MAINGAME_PROCESS;
    memcpy(msg->data,&SongState,sizeof(SongState));
    SetSidx(sset.song_mode,sset.p1_songid, sset.p2_songid);
    printf("SoundIndex Events: %d\n",current_sidx_length);
    SongState.p1_cursor[0].cursor_location = 0x100;
    song_playing = 1;
    //pthread_create(&bgm_hthread, 0, bgm_thread, 0);
   
}








void Update_Cursors(){
    if(cursor_delay != cursor_delay_max){
        cursor_delay++;
        return;
    }
   
    if(SongState.p1_playing){
        for(int i = 0; i < 150; i++){        
            if(SongState.p1_cursor[i].cursor_type){
                SongState.p1_cursor[i].cursor_location+=4;
                if(SongState.p1_cursor[i].cursor_location >= 0x1A0){
                 // Cull after this amount.
                SongState.p1_cursor[i].cursor_idk = 0;
                SongState.p1_cursor[i].cursor_location = 0;
                SongState.p1_cursor[i].cursor_type = 0;
                }
            }            
        }
        // Add Ones from Event if Needed
        for(int j = 0; j < num_cursor_events;j++){
            PNCEVT ce = (PNCEVT)(cursor_event_table + (j*sizeof(NCEVT)));
            if(ce->note_index != SongState.p1_note_counter){continue;}
            SongState.p1_cursor[ce->cursor_slot].cursor_location = 0;
            SongState.p1_cursor[ce->cursor_slot].cursor_idk = ce->cursor_idk;
            SongState.p1_cursor[ce->cursor_slot].cursor_type = ce->cursor_type;
        }
    }

    if(SongState.p2_playing){
        for(int i = 0; i < 150; i++){        
            if(SongState.p2_cursor[i].cursor_type){
                SongState.p2_cursor[i].cursor_location+=4;
                if(SongState.p1_cursor[i].cursor_location >= 0x1A0){
                 // Cull after this amount.
                SongState.p1_cursor[i].cursor_idk = 0;
                SongState.p1_cursor[i].cursor_location = 0;
                SongState.p1_cursor[i].cursor_type = 0;
                }                
            }
        }
        // Add Ones from Event if Needed
        for(int j = 0; j < num_cursor_events;j++){
            PNCEVT ce = (PNCEVT)(cursor_event_table + (j*sizeof(NCEVT)));
            if(ce->note_index != SongState.p2_note_counter){continue;}
            SongState.p2_cursor[ce->cursor_slot].cursor_location = 0;
            SongState.p2_cursor[ce->cursor_slot].cursor_idk = ce->cursor_idk;
            SongState.p2_cursor[ce->cursor_slot].cursor_type = ce->cursor_type;
        }
    }
    
   
    
}


void Song_MainGameProcess(const unsigned char* in_data, struct A27_Read_Message* msg){
   
    if(note_tick == note_tick_max){
        note_tick = 0;
        current_note++;
    }

       SongState.cmd = A27_SONGMODE_MAINGAME_PROCESS;
    if(SongState.p1_playing){
        SongState.p1_note_counter = current_note;
    }
    if(SongState.p2_playing){
        SongState.p2_note_counter = current_note;
    }
    
   
    
    unsigned int sw = KeyIO_GetSwitches();
    unsigned int lastst = msg->button_io[0];
    // Set Lane Animation
    SongState.p1_lane_animation[ANI_BLUE] = IO_ISSSET(sw,last_sw,INP_P1_BLUE) ? 1:0;
   // if(SongState.p1_lane_animation[ANI_BLUE]){SoundMgr_Play(GetSoundSlot(KEYSOUND_BLUE),0);}
    SongState.p1_lane_animation[ANI_DRUM_L] = IO_ISSSET(sw,last_sw,INP_P1_DRUM_L) ? 1:0;
    SongState.p1_lane_animation[ANI_DRUM_R] = IO_ISSSET(sw,last_sw,INP_P1_DRUM_R) ? 1:0;
    SongState.p1_lane_animation[ANI_RIM_L] = IO_ISSSET(sw,last_sw,INP_P1_RIM_L) ? 1:0;
    SongState.p1_lane_animation[ANI_RIM_R] = IO_ISSSET(sw,last_sw,INP_P1_RIM_R) ? 1:0;
    SongState.p1_lane_animation[ANI_RED] = IO_ISSSET(sw,last_sw,INP_P1_RED) ? 1:0;    
    
    SongState.p2_lane_animation[ANI_BLUE] = IO_ISSSET(sw,last_sw,INP_P2_BLUE) ? 1:0;
    SongState.p2_lane_animation[ANI_DRUM_L] = IO_ISSSET(sw,last_sw,INP_P2_DRUM_L) ? 1:0;
    SongState.p2_lane_animation[ANI_DRUM_R] = IO_ISSSET(sw,last_sw,INP_P2_DRUM_R) ? 1:0;
    SongState.p2_lane_animation[ANI_RIM_L] = IO_ISSSET(sw,last_sw,INP_P2_RIM_L) ? 1:0;
    SongState.p2_lane_animation[ANI_RIM_R] = IO_ISSSET(sw,last_sw,INP_P2_RIM_R) ? 1:0;
    SongState.p2_lane_animation[ANI_RED] = IO_ISSSET(sw,last_sw,INP_P2_RED) ? 1:0;    
   
    // Keysound Setting -- Note: Technically the card puts the effect in the first available slot
    // Because we have plenty of open slots all the time, it's not a huge deal to dedicate a slot and skip that step.
    // Maybe it's every other frame? 
    // Maybe the keysounds should be only added to the queue from the last frame, we won't add them to the current frame.
    
  
    
   
    SongState.sound_index[1] = IO_ISSSET(sw,last_sw,INP_P1_RED) ? KEYSOUND_RED:0;   
    SongState.sound_index[8] = IO_ISSSET(sw,last_sw,INP_P2_BLUE) ? KEYSOUND_BLUE:0;
    SongState.sound_index[9] = IO_ISSSET(sw,last_sw,INP_P2_RED) ? KEYSOUND_RED:0;  
    
    if((IO_ISSSET(sw,last_sw,INP_P1_DRUM_L) || IO_ISSSET(sw,last_sw,INP_P1_DRUM_R)) && !SongState.sound_index[2]){
       SongState.sound_index[2] = KEYSOUND_DRUM;   
        if((IO_ISSSET(sw,lastst,INP_P1_DRUM_L) && IO_ISSSET(sw,lastst,INP_P1_DRUM_R))){
           SongState.sound_index[2] = KEYSOUND_2DRUM; 
        }
    }else{
     SongState.sound_index[2] = 0;   
    }
    
    
    Update_Cursors();
        // Red is at 368 - 0x170,  great cool nice poor 
        // 0 is when the first pixel appears, but -1 works to keep it offscreen
        // 0x1AE is when it's basically offscreen and can be culled
    // 367 to 371 should be great
    // 372 382 and 357 to 367 should be cool 
    // 348     391 should be nice
    // anything else is poor 
    
    
    /*
    if((IO_ISSET(sw,INP_P1_RIM_L) && IO_ISSET(sw,INP_P1_RIM_R)) || (IO_ISSET(sw,INP_P1_RIM_L) && IO_ISSET(last_sw,INP_P1_RIM_R)) || (IO_ISSET(sw,INP_P1_RIM_R) && IO_ISSET(last_sw,INP_P1_RIM_L))){
        SongState.sound_index[2] = KEYSOUND_2RIM;
    }else if(IO_ISSSET(sw,last_sw,INP_P1_RIM_L) || IO_ISSSET(sw,last_sw,INP_P1_RIM_R)){
        SongState.sound_index[2] = KEYSOUND_RIM;
    }

 
    if(IO_ISSET(sw,INP_P2_DRUM_L) && IO_ISSET(sw,INP_P2_DRUM_R)){
        SongState.sound_index[10] = KEYSOUND_2DRUM;
    }else if(IO_ISSSET(sw,last_sw,INP_P2_DRUM_L) || IO_ISSSET(sw,last_sw,INP_P2_DRUM_R)){
        SongState.sound_index[10] = KEYSOUND_DRUM;
    }
    if(IO_ISSET(sw,INP_P2_RIM_L) && IO_ISSET(sw,INP_P2_RIM_R)){
        SongState.sound_index[10] = KEYSOUND_2RIM;
    }else if(IO_ISSSET(sw,last_sw,INP_P2_RIM_L) || IO_ISSSET(sw,last_sw,INP_P2_RIM_R)){
        SongState.sound_index[10] = KEYSOUND_RIM;
    }
    */
    

/*
 #pragma pack(1)
typedef struct _CURSOR_STATE{
    unsigned int cursor_type;
    unsigned short cursor_location;
    unsigned char idk_5;
    unsigned char idk_6;
}NOTECURSOR;
 */

/*
 Index: 2 Cursor: 82400000 4f00 0000
Index: 3 Cursor: 02 40 40 00 4f00 00 00
Index: 5 Cursor: 42400026 9e01 e8 fe

 */

  //  SongState.p1_cursor[2].cursor_type = 0x4082;
 //   SongState.p1_cursor[2].cursor_location = 0x4F;
    
    //SongState.p1_cursor[3].cursor_type = 0x40C002;
   // SongState.p1_cursor[3].cursor_location = 0xE0;
    // 2nd bit of 4th (last) byte controls the holds 
    // each hit is stored in a bit after that... so bit 3 will make it 6 (combo 1), bit 4 will make it 0x0A (combo 2), but 10110 (0x16) is 5 because we're basically ignoring the 2 bits at the beginning and focusing on whats left for the combo tracking 
    // 0xFE is probably the highest at 63 - this is the hit continuously one... it's not the certain number of hits... that's another type.
    //SongState.p1_cursor[5].cursor_type = 0x2600c042;
    //SongState.p1_cursor[5].cursor_location = 0x150;
    //SongState.p1_cursor[5].cursor_idk = 0xFE20;
    
    
    
    //SongState.p1_cursor[0].cursor_type = IO_ISSSET(sw,last_sw,INP_P1_BLUE) ? 0x26004042 : 0x00004042;
  //  if(IO_ISSSET(sw,last_sw,INP_P1_DRUM_R)){
  //  SongState.p1_cursor[5].cursor_idk = 0xFEe1;
  //  }
    
 //   SongState.idk_maybepadding[0] = IO_ISSSET(sw,last_sw,INP_P1_DRUM_R) ? 9:0;
    
 //   SongState.p1_cursor[1].cursor_type = 0x4040;
    // Let's test out some bullshit for scoring and life stuff
    //SongState.p1_judge_animation[ANI_BLUE] = 0;
    //if(note_tick == 5){
        if(IO_ISSSET(sw,last_sw,INP_P1_RED)){
         SongState.p1_combo++;
         SongState.p1_judge_animation[ANI_RED] = ANI_JUDGE_GREAT;
         SongState.p1_lifebar+=1;
         SongState.p1_score+=30;
   //      SongState.p1_cursor[0].cursor_location++;
       //  printf("Cursor: %d\n",SongState.p1_cursor[0].cursor_location);
        }
        if(IO_ISSSET(sw,last_sw,INP_P1_BLUE)){
         SongState.p1_combo = 0;
         SongState.p1_judge_animation[ANI_BLUE] = ANI_JUDGE_POOR;
         SongState.p1_lifebar-=1;
    //     SongState.p1_cursor[0].cursor_location--;
    //     printf("Cursor: %d\n",SongState.p1_cursor[0].cursor_location);
        }        
   // }
    
    
    /*
     idk1 
     0x02 -  Blue Drum
     0x42 -  Single Drum
     0x82 -  Double Drum
     0xC2 -  Single rim
     0x102 - Double Rim
     0x142 - Red Drum
     0x182 - Offscreen Sparkly
     0x1C2 - Offscreen Sparkly
     
     cursor link is actually more style bytes - 0x40 makes it a bar measure
     */
    
     Update_Soundindex();
    msg->dwBufferSize = 0xA00;

    // TODO: All the Gameplay Logic Stuff
    // At some point, the game can call Mode 7 in a response for this. - I think it interrupts the song and goes to the results.
       memcpy(msg->data,&SongState,sizeof(SongState));
       last_sw = sw;
       note_tick++;
}

    
void Song_ResultProcess(const unsigned char* in_data, struct A27_Read_Message* msg){
  
    SONGRESULT_REQUEST* req = (SONGRESULT_REQUEST*)in_data;
    SongResult sres = {0};
    sres.cmd = A27_SONGMODE_RESULT;
    sres.p1_num_notes = 123;
    sres.p2_num_notes = 456;

    sres.p1_great = 1;
    sres.p1_cool = 2;
    sres.p1_nice = 3;
    sres.p1_poor = 4;
    sres.p1_lost = 5;
    sres.song_clear = 1;
    sres.enable_bonus_stage = 1;

    sres.p2_great = 1;
    sres.p2_cool = 2;
    sres.p2_nice = 3;
    sres.p2_poor = 4;
    sres.p2_lost = 5;
    sres.p1_max_combo = 6;
    sres.p2_max_combo = 6;
    sres.p1_fever_beat = 7;
    sres.p2_fever_beat = 7;

    
    sres.p1_score = 61;
    sres.p2_score = 60;
    sres.p1_grade = 0; 
    sres.p2_grade = 1; 

    // TODO: Calculate Grade
    
    // Calculating Message 
    if(sres.p1_num_notes == sres.p1_max_combo){
        sres.p1_message = SongResulMessage_BRAVO;
        if(sres.p1_max_combo == sres.p1_cool){
            sres.p1_message = SongResulMessage_PERFECT;   
        }
    }

    if(sres.p2_num_notes == sres.p2_max_combo){
        sres.p2_message = SongResulMessage_BRAVO;
        if(sres.p2_max_combo == sres.p1_cool){
            sres.p2_message = SongResulMessage_PERFECT;   
        }
    }
    
    msg->dwBufferSize = sizeof(SongResult);
    memcpy(msg->data,&sres,sizeof(SongResult));
    song_playing = 0;
}

