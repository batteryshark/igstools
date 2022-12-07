// A27 PercussionMaster SongMode Process Module
#include <stdio.h>
#include <string.h>

#include "../utils.h"
#include "a27.h"
#include "song.h"

static struct maingame_setting sset = {0x00};
static unsigned char playback_header[148] = {0x00};
static unsigned char playback_body[4004] = {0x00};

static short current_note;
static unsigned char note_tick_max = 6;
static unsigned char note_tick = 0;

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
    unsigned char playerstate_idk[4]; // 0x9ac
    unsigned char p1_playing; // 0x9b0
    unsigned char p2_playing; // 0x9b1
    unsigned char playerstate_idk2[2];
    unsigned char p1_judge[6]; // 0x9b4
    unsigned char playerstate_idk3[42];
    unsigned int p1_score; // 0x9e4
    unsigned int p2_score; // 0x9e8
    unsigned int p1_score_2; // 0x9ec
    unsigned int p2_score_2; // 0x9f0
    unsigned char playerstate_idk4[4]; // 0x9f4
    unsigned short bg_index[4]; // 0x9f8
}SongState;



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

void Song_MainGameSetting(const unsigned char* in_data, struct A27_Read_Message* msg){
    memcpy(&sset,in_data,sizeof(sset));
    print_song_setting();

    memset(&SongState,0,sizeof(SongState));
    SongState.cmd = A27_SONGMODE_MAINGAME_SETTING;

    memcpy(msg->data,&SongState,sizeof(SongState));
    msg->dwBufferSize = sizeof(SongState);
    printf("Packet: %04X\n",sizeof(SongState));
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
}

void Song_MainGameProcess(const unsigned char* in_data, struct A27_Read_Message* msg){
    note_tick++;
    if(note_tick == note_tick_max){
        note_tick = 0;
        current_note+=100;
    }
    // Reset sound_index
    memset(SongState.sound_index,0,sizeof(SongState.sound_index));
       SongState.cmd = A27_SONGMODE_MAINGAME_PROCESS;
    if(SongState.p1_playing){
        SongState.p1_note_counter = current_note;
    }
    if(SongState.p2_playing){
        SongState.p2_note_counter = current_note;
    }

    if(current_note == 10 && note_tick == 0){
        if(sset.song_mode != 0){
        SongState.sound_index[16] = 7;    
        }else{
        SongState.sound_index[16] = 1;    
        }
        
    }
    msg->dwBufferSize = 0xA00;

    // TODO: All the Gameplay Logic Stuff
    // At some point, the game can call Mode 7 in a response for this. - I think it interrupts the song and goes to the results.
       memcpy(msg->data,&SongState,sizeof(SongState));
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

