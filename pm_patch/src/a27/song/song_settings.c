#include <stdio.h>
#include <string.h>

#include "song_settings.h"

// Debug Helpers
void PrintSongSetting(PSongSettings song_setting){
    printf("--- New Song Setting ---\n");

    printf("[Song Settings]\n");
    printf("Stage: %d GameMode: %d Record Mode:%d\n",song_setting->stage_num,song_setting->game_mode,song_setting->key_record_mode);    
    printf("Non Challenge Mode: %d\n",song_setting->is_non_challengemode);
    printf("Song Mode: %d\n",song_setting->song_mode);
    printf("Judge Margins: G: %d C: %d N: %d P: %d\n",song_setting->judge.great,song_setting->judge.cool,song_setting->judge.nice,song_setting->judge.poor);    
    
    for(int i=0;i<2;i++){
        printf("[Player %d Settings]\n", i+1);
        printf("Enabled: %d SongVersion: %d Chart ID: %d Rating: %d\n",song_setting->player_enable[i],song_setting->player_song_version[i],song_setting->player_chartid[i],song_setting->player_rating_level[i]);
        printf("Speed: %d Cloak: %d Noteskin: %d Autoplay: %d\n",song_setting->player_mod[i].speed,song_setting->player_mod[i].cloak,song_setting->player_mod[i].noteskin,song_setting->player_autoplay[i]);
        printf("FEVER: %.2f GREAT: %.2f COOL: %.2f NICE: %.2f POOR: %.2f MISS: %.2f\n",song_setting->level_rate[i].fever,song_setting->level_rate[i].great,song_setting->level_rate[i].cool,song_setting->level_rate[i].nice,song_setting->level_rate[i].poor,song_setting->level_rate[i].miss);
    }
    printf("--- End Song Setting ---\n");
}


