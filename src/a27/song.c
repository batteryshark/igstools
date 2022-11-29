// A27 PercussionMaster SongMode Process Module
#include <stdio.h>
#include <string.h>

#include "../utils.h"
#include "a27.h"
#include "song.h"

static struct maingame_setting sset = {0x00};
static unsigned char playback_header[148] = {0x00};
static unsigned char playback_body[4004] = {0x00};

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
    printf("LevelRate P1: %.2f %.2f %.2f %.2f %.2f %.2f\n",sset.level_rate_p1[0],sset.level_rate_p1[1],sset.level_rate_p1[2],sset.level_rate_p1[3],sset.level_rate_p1[4],sset.level_rate_p1[5]);
    printf("LevelRate P2: %.2f %.2f %.2f %.2f %.2f %.2f\n",sset.level_rate_p2[0],sset.level_rate_p2[1],sset.level_rate_p2[2],sset.level_rate_p2[3],sset.level_rate_p2[4],sset.level_rate_p2[5]);
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
    msg->dwBufferSize = 0xA00;
    memset(msg->data,0,msg->dwBufferSize);
    *(unsigned short*)msg->data = A27_SONGMODE_MAINGAME_SETTING;
}

void Song_MainGameWaitStart(const unsigned char* in_data, struct A27_Read_Message* msg){
    msg->dwBufferSize = 0xA00;
    memset(msg->data,0,msg->dwBufferSize);
    *(unsigned short*)msg->data = A27_SONGMODE_MAINGAME_WAITSTART;    
}

void Song_MainGameStart(const unsigned char* in_data, struct A27_Read_Message* msg){
    msg->dwBufferSize = 0xA00;
    *(unsigned short*)msg->data = A27_SONGMODE_MAINGAME_PROCESS;    
}

void Song_MainGameProcess(const unsigned char* in_data, struct A27_Read_Message* msg){
    msg->dwBufferSize = 0xA00;
    *(unsigned short*)msg->data = A27_SONGMODE_MAINGAME_PROCESS;
    
    // TODO: All the Gameplay Logic Stuff
    // At some point, the game can call Mode 7 in a response for this. - I think it interrupts the song and goes to the results.
    
}

void Song_ResultProcess(const unsigned char* in_data, struct A27_Read_Message* msg){
    *(unsigned short*)msg->data = A27_SONGMODE_RESULT;
    msg->dwBufferSize = 80;
    msg->data[0x4C] = 3;
    msg->data[0x4D] = 3;
    msg->data[0x4E] = 2;
    msg->data[0x4F] = 2;
    // TODO: Figure out what this is for.
}

