// A27 PercussionMaster SongMode Process Module
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "../keyio.h"
#include "../utils.h"
#include "a27.h"
#include "song.h"

static struct maingame_setting sset = {0x00};
static unsigned char playback_header[148] = {0x00};
static unsigned char playback_body[4004] = {0x00};



enum JUDGEPROC{
    GREAT=1,
    COOL,
    NICE,
    POOR,
    LOST, // Battle Mode
    BRAVO, // Battle Mode
    IDK1,
    IDK2,
    IDK3
};

#pragma pack(1)
typedef struct _CURSOR_STATE{
    unsigned char idk_1;
    unsigned char idk_2;
    unsigned char idk_3;
    unsigned char idk_4;
    unsigned short cursor_location;
    unsigned char idk_5;
    unsigned char idk_6;
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
    unsigned short bg_index[4]; 
}SongState;



static short current_note;

static unsigned char note_tick_max = 6;
static unsigned char note_tick = 0;





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

typedef struct _SOUNDINDEX_EVENT{
    unsigned short note_index;
    unsigned char  sound_index;
    unsigned short sound_value;
}SIDXEVT;

static SIDXEVT opening_sidx[] = {
    {10,16,1}   
};

static SIDXEVT howtoplay_sidx[] = {
    
    {15,16,7},
{55,17,8},
{103,17,9},
{167,17,10},
{231,17,11},
{279,17,12},
{367,17,3},
{407,17,13},
{486,17,4},
{519,17,14},
{607,17,5},
{639,17,15},
{727,17,6},
{767,17,16},
{847,17,1},
{879,17,17},
{967,17,2},
{1079,17,1},
{1079,18,2},
{1095,17,18},
{1143,17,19},
{1157,17,3},
{1161,17,3},
{1165,17,3},
{1169,17,3},
{1173,17,3},
{1177,17,3},
{1215,17,20},
{1229,17,4},
{1233,17,4},
{1237,17,4},
{1241,17,4},
{1245,17,4},
{1249,17,4},
{1287,17,21},
{1297,17,1},
{1301,17,1},
{1305,17,1},
{1309,17,1},
{1313,17,1},
{1317,17,1},
{1321,17,1},
{1351,17,22},
{1368,17,2},
{1372,17,2},
{1376,17,2},
{1380,17,2},
{1384,17,2},
{1388,17,2},
{1392,17,2},
{1407,17,23},
{1499,17,3},
{1503,17,3},
{1507,17,3},
{1511,17,3},
{1515,17,3},
{1519,17,3},
{1523,17,3},
{1527,17,3},
{1531,17,3},
{1535,17,3},
{1539,17,3},
{1543,17,3},
{1547,17,3},
{1551,17,3},
{1555,17,3},
{1609,17,4},
{1613,17,4},
{1617,17,4},
{1621,17,4},
{1625,17,4},
{1629,17,4},
{1633,17,4},
{1637,17,4},
{1641,17,4},
{1645,17,4},
{1663,17,24},
{1794,17,1},
{1799,17,1},
{1807,17,3},
{1815,17,25},
{1816,17,2}
    
};

static SIDXEVT* current_sidx;
static unsigned int current_sidx_length;

void SetSidx(unsigned short song_mode, unsigned short p1_songid, unsigned short p2_songid){
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
}


void Song_MainGameSetting(const unsigned char* in_data, struct A27_Read_Message* msg){
    memcpy(&sset,in_data,sizeof(sset));
    print_song_setting();

    memset(&SongState,0,sizeof(SongState));
    SongState.cmd = A27_SONGMODE_MAINGAME_SETTING;

    memcpy(msg->data,&SongState,sizeof(SongState));
    msg->dwBufferSize = sizeof(SongState);

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
    
    SongState.bg_index[0] = 10; // I don't know what this is for.
    SongState.bg_index[1] = 10;
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
   
}


unsigned int last_sw;



void Update_Soundindex(){
    
    // Reset sound_index
    memset(SongState.sound_index,0,sizeof(SongState.sound_index));
    for(int i = 0; i < current_sidx_length;i++){
        if(current_sidx[i].note_index < 1){continue;};
        if(current_sidx[i].note_index == current_note){
            SongState.sound_index[current_sidx[i].sound_index] = current_sidx[i].sound_value;
            // We're effectively nuking this item to not be played again.
            current_sidx[i].note_index = 0;
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
    
    Update_Soundindex();
    
    unsigned int sw = KeyIO_GetSwitches();
    
    // Set Lane Animation
    SongState.p1_lane_animation[ANI_BLUE] = IO_ISSSET(sw,last_sw,INP_P1_BLUE) ? 1:0;
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
   
    SongState.sound_index[0] = IO_ISSSET(sw,last_sw,INP_P1_DRUM_L) ? 1:0;
    SongState.sound_index[1] = IO_ISSSET(sw,last_sw,INP_P1_DRUM_R) ? 2:0;
    SongState.sound_index[2] = IO_ISSSET(sw,last_sw,INP_P1_RIM_R) ? 3:0;
    SongState.sound_index[3] = IO_ISSSET(sw,last_sw,INP_P1_RIM_L) ? 4:0;
    SongState.sound_index[4] = IO_ISSSET(sw,last_sw,INP_P1_BLUE) ? 5:0;
    SongState.sound_index[5] = IO_ISSSET(sw,last_sw,INP_P1_RED) ? 6:0;

    

    
    
    
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
}

