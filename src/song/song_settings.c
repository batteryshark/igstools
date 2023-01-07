#include <stdio.h>
#include <string.h>

#include "song_settings.h"

// Debug Helpers
void PrintSongSetting(PSongSettings song_setting){
    printf("--- New Song Setting ---\n");
    printf("State: %d\n",song_setting->state);
    printf("Stage: %d GameMode: %d KeyRecord:%d\n",song_setting->stage_num,song_setting->game_mode,song_setting->key_record_mode);
    printf("P1 Enable: %d Version: %d SongID: %d\n",song_setting->p1_enable,song_setting->p1_songversion,song_setting->p1_songid);
    printf("P1 Speed: %d Cloak: %d Noteskin: %d Auto: %d\n",song_setting->p1_speed,song_setting->p1_cloak,song_setting->p1_noteskin,song_setting->p1_autoplay);
    printf("P2 Enable: %d Version: %d SongID: %d\n",song_setting->p2_enable,song_setting->p2_songversion,song_setting->p2_songid);
    printf("P2 Speed: %d Cloak: %d Noteskin: %d Auto: %d\n",song_setting->p2_speed,song_setting->p2_cloak,song_setting->p2_noteskin,song_setting->p2_autoplay);
    printf("Scoring: G: %d C: %d N: %d P: %d\n",song_setting->judge_great,song_setting->judge_cool,song_setting->judge_nice,song_setting->judge_poor);
    printf("P1 Rating: %d P2 Rating: %d\n",song_setting->p1_rating,song_setting->p2_rating);
    printf("LevelRate P1: Fever %.2f Great %.2f Cool %.2f Nice %.2f Poor %.2f Miss %.2f\n",song_setting->level_rate_p1[0],song_setting->level_rate_p1[1],song_setting->level_rate_p1[2],song_setting->level_rate_p1[3],song_setting->level_rate_p1[4],song_setting->level_rate_p1[5]);
    printf("LevelRate P2: Fever %.2f Great %.2f Cool %.2f Nice %.2f Poor %.2f Miss %.2f\n",song_setting->level_rate_p2[0],song_setting->level_rate_p2[1],song_setting->level_rate_p2[2],song_setting->level_rate_p2[3],song_setting->level_rate_p2[4],song_setting->level_rate_p2[5]);
    printf("IDK 1 [Probably Padding]: ");
    for(int i=0;i<8;i++){
     printf("%02X",song_setting->idk_1[i]);   
    }
    printf("\n");
    printf("IDK P1: %d P2: %d\n",song_setting->idk_p1_1,song_setting->idk_p1_2);
    printf("Is Non Challenge Mode: %d\n",song_setting->is_non_challengemode);
    printf("Song Mode: %d\n",song_setting->song_mode);
    printf("--- End Song Setting ---\n");
}


