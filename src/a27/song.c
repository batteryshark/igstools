#include <stdio.h>
#include <string.h>


#include "../utils.h"

#include "a27.h"

#include "song_utils.h"
#include "song.h"


// Globals
static SongSetting song_setting = {0x00};
static SongState   song_state = {0x00};

// Debug Helpers
void PrintSongSetting(void){
    printf("--- New Song Setting ---\n");
    printf("State: %d\n",song_setting.state);
    printf("Stage: %d GameMode: %d KeyRecord:%d\n",song_setting.stage_num,song_setting.game_mode,song_setting.key_record_mode);
    printf("P1 Enable: %d Version: %d SongID: %d\n",song_setting.p1_enable,song_setting.p1_songversion,song_setting.p1_songid);
    printf("P1 Speed: %d Cloak: %d Noteskin: %d Auto: %d\n",song_setting.p1_speed,song_setting.p1_cloak,song_setting.p1_noteskin,song_setting.p1_autoplay);
    printf("P2 Enable: %d Version: %d SongID: %d\n",song_setting.p2_enable,song_setting.p2_songversion,song_setting.p2_songid);
    printf("P2 Speed: %d Cloak: %d Noteskin: %d Auto: %d\n",song_setting.p2_speed,song_setting.p2_cloak,song_setting.p2_noteskin,song_setting.p2_autoplay);
    printf("Scoring: G: %d C: %d N: %d P: %d\n",song_setting.judge_great,song_setting.judge_cool,song_setting.judge_nice,song_setting.judge_poor);
    printf("P1 Rating: %d P2 Rating: %d\n",song_setting.p1_rating,song_setting.p2_rating);
    printf("LevelRate P1: Fever %.2f Great %.2f Cool %.2f Nice %.2f Poor %.2f Lost %.2f\n",song_setting.level_rate_p1[0],song_setting.level_rate_p1[1],song_setting.level_rate_p1[2],song_setting.level_rate_p1[3],song_setting.level_rate_p1[4],song_setting.level_rate_p1[5]);
    printf("LevelRate P2: Fever %.2f Great %.2f Cool %.2f Nice %.2f Poor %.2f Lost %.2f\n",song_setting.level_rate_p2[0],song_setting.level_rate_p2[1],song_setting.level_rate_p2[2],song_setting.level_rate_p2[3],song_setting.level_rate_p2[4],song_setting.level_rate_p2[5]);
    printf("IDK 1 [Probably Padding]: ");
    PrintHex(song_setting.idk_1,8);
    printf("IDK P1: %d P2: %d\n",song_setting.idk_p1_1,song_setting.idk_p1_2);
    printf("Is Non Challenge Mode: %d\n",song_setting.is_non_challengemode);
    printf("Song Mode: %d\n",song_setting.song_mode);
    printf("--- End Song Setting ---\n");
}


// -- Unimplemented Function 0 --
void Song_UploadPlaybackHeader(const unsigned char* in_data, struct A27_Read_Message* msg){
    // TODO: Upload Logic
    // This Effectively means OK?
    msg->dwBufferSize = 4;
    msg->data[0] = 2;
}

// -- Unimplemented Function 1 --
void Song_UploadPlaybackBody(const unsigned char* in_data, struct A27_Read_Message* msg){    
    // Do Nothing for Now
    // TODO: Upload Logic
    // This Effectively means OK?
    msg->dwBufferSize = 4;
    msg->data[0] = 2;
}

// -- Function 3: This tells the card what song we're playing and sets our session options.
// -- This is called twice, so don't do any one-shot stuff in here.
void Song_MainGameSetting(const unsigned char* in_data, struct A27_Read_Message* msg){
    memcpy(&song_setting,in_data,sizeof(song_setting));
    memset(&song_state,0,sizeof(song_state));
    song_state.cmd = A27_SONGMODE_MAINGAME_SETTING;
    memcpy(msg->data,&song_state,sizeof(song_state));
    msg->dwBufferSize = sizeof(song_state);
}

// -- Function 4: This is when we start initializing the song state stuff like setting note counters, etc.
void Song_MainGameWaitStart(const unsigned char* in_data, struct A27_Read_Message* msg){
    msg->dwBufferSize = sizeof(song_state);

    song_state.cmd = A27_SONGMODE_MAINGAME_WAITSTART;
    
    if(song_setting.p1_enable){
            song_state.p1_playing = 1;
            song_state.p1_note_counter = -1;
            song_state.p1_lifebar = 10;
    }
    if(song_setting.p2_enable){
            song_state.p2_playing = 1;
            song_state.p2_note_counter = -1;
            song_state.p2_lifebar = 10;
    }    
    
    memset(msg->data,0,sizeof(song_state));
    *(unsigned short*)msg->data = A27_SONGMODE_MAINGAME_WAITSTART;
}

// -- Function 5: This is a one-shot function that starts the song timer and is likely the official start of the song.
void Song_MainGameStart(const unsigned char* in_data, struct A27_Read_Message* msg){
    msg->dwBufferSize = sizeof(song_state);
    song_state.cmd = A27_SONGMODE_MAINGAME_PROCESS;
    short current_beat = GetCurrentBeat();
    if(song_state.p1_playing){
        song_state.p1_note_counter = current_beat;
    }
    if(song_state.p2_playing){
        song_state.p2_note_counter = current_beat;
    }
	
    memcpy(msg->data,&song_state,sizeof(song_state));
    PrintSongSetting();
    // At this point, we can start our song timer.
    StartSong(song_setting.song_mode,song_setting.p1_songid, song_setting.p2_songid);
 
}

// -- Function 6: This is called every frame to update the song state
void Song_MainGameProcess(const unsigned char* in_data, struct A27_Read_Message* msg){
    msg->dwBufferSize = sizeof(song_state);
    song_state.cmd = A27_SONGMODE_MAINGAME_PROCESS;
    short current_beat = GetCurrentBeat();
    if(song_state.p1_playing){
        song_state.p1_note_counter = current_beat;
    }
    if(song_state.p2_playing){
        song_state.p2_note_counter = current_beat;
    }
    
    
    // Fire Sound Cues if necessary.
    GetSoundIndex(song_state.sound_index);
    GetCursorState(0,song_state.p1_cursor);
    GetCursorState(1,song_state.p2_cursor);
    
    
    
    // TODO: Capture Current Input for Next Frame Judgement
    
    // TODO: Update Player Combos, Scoring
    /*
     A Note on scoring: From what I remember, a single "Great" is 5 points, then every additional great is 5*combo. so 10,15,20 etc
     I think as long as you don't break the combo, that follows - cool might be 3 * combo, nice = 1*combo... something similar to that
     */
    // On beat zero, I have to start a new "measure" cursor.
    // On beat 8, the measure cursor should be directly at 0x170
    // 
    
    
    
    // TODO: Based on previous frame, light up the various lanes if hit and react to notes.
    
    
    
    // TODO: All State Update Shit
    memcpy(msg->data,&song_state,sizeof(song_state));
    
    // Reset Sound Index Back to Blank State for Next Frame
    ClearSoundIndex();
    
    // TODO: Based on previous frame, cue up sound effects (keysounds) for next frame
    
}

// -- Function 9: This stops the song and goes to results. I think Function 7 also does that but we'll deal later.
void Song_ResultProcess(const unsigned char* in_data, struct A27_Read_Message* msg){
    printf("Number of Beats: %d\n",GetCurrentBeat());
    StopSong();
    
    SongResult song_result = {0};
    GetSongResult((PSongResultRequest)in_data,&song_result);
    
    msg->dwBufferSize = sizeof(SongResult);
    memcpy(msg->data,&song_result,sizeof(SongResult));
}


 
