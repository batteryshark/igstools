// Handling Code for PM1 Recfile Reading
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "song_recfile.h"

#define RECFILE_MAGIC "PM1RECFILE  00.2004.0001"
// These are hardcoded so no worries on that.
#define OFFSET_REC_P1_CURSOR 0x0196
#define OFFSET_REC_P2_CURSOR 0x7EC2
#define OFFSET_REC_SOUND     0xFBEE

unsigned short (*song_wSongNameGet)(unsigned short chart_id) = (void*)0x08077188;
unsigned char (*song_bSongRankGet)(unsigned short chart_id) = (void*)0x080771A0;
const char** g_pchaSongName = (void*)0x08171CE0;
const char** g_pchaSongRank = (void*)0x08171D94;

int GenerateEventFilename(unsigned int song_mode, unsigned int chart_id, char* event_file_path){
    switch(song_mode){
     case SONG_MODE_OPENING:
         strcat(event_file_path,"Opening.rec");
         return 1;
     case SONG_MODE_HOW_TO_PLAY:
         strcat(event_file_path,"HowToPlay.rec");
         return 1;
     case SONG_MODE_STAFF:
         strcat(event_file_path,"Staff.rec");
         return 1;
     default:
         break;     
    }
    strcat(event_file_path,g_pchaSongName[song_wSongNameGet(chart_id)]);
    strcat(event_file_path,"_");
    strcat(event_file_path,g_pchaSongRank[song_bSongRankGet(chart_id)]);
    strcat(event_file_path,".rec"); 
    return 1;
}

int LoadRecfile(const char* path_to_recfile,PRecFile rec_file){
    memset(rec_file,0,sizeof(RecFile));
    printf("Opening SongData (Rec) File: %s\n",path_to_recfile);
    FILE* fp = fopen(path_to_recfile,"rb");
    if(fp == NULL){
        printf("Error: Opening RecFile: %s\n",path_to_recfile);
        return 0;
    }
    
    // Read File Header Data and Check Magic
    fread(&rec_file->header,sizeof(RecFileHeader),1,fp);
    if(memcmp(rec_file->header.magic,RECFILE_MAGIC,strlen(RECFILE_MAGIC))){
        printf("Error: Invalid RecFile: %s\n",path_to_recfile);
        fclose(fp);
        return 0;
    }
    
    // Load the Player 1 Cursor Events    
    fseek(fp,OFFSET_REC_P1_CURSOR,SEEK_SET);
    fread(rec_file->p1_events,sizeof(RecFileLane)*8,1,fp);
    
    // Load the Player 2 Cursor Events
    fseek(fp,OFFSET_REC_P2_CURSOR,SEEK_SET);
    fread(rec_file->p2_events,sizeof(RecFileLane)*8,1,fp);
    
    // Load the Sound Events
    fseek(fp,OFFSET_REC_SOUND,SEEK_SET);
    fread(rec_file->sound_events,sizeof(RecFileLane)*8,1,fp);        

    fclose(fp);

    return 1;
}



// From the A27 Header Data - Load our Rec Structure.
int LoadRecHeader(PRecDataHeader rec_header_data, PRecFile rec_file){
    memset(rec_file,0,sizeof(RecFile));
    // The first part of this is basically the rec header with a different beginning.
    memcpy((unsigned char*)&rec_file->header+28,(unsigned char*)rec_header_data+2,sizeof(RecDataHeader)-2);
    for(int i=0;i<8;i++){
            rec_file->p1_events[i].num_events = rec_header_data->p1_track_total_events[i]-1;
            rec_file->p2_events[i].num_events = rec_header_data->p2_track_total_events[i]-1;
            rec_file->sound_events[i].num_events = rec_header_data->sound_track_total_events[i]-1;
    }
    return 1;
}
int LoadRecData(PA27RecDataBody rec_data, PRecFile rec_file){
    unsigned int event_offset = 0;
    // Load Player 1 Events    
    for(int i=0;i<8;i++){
        unsigned dest_slot = 0;
        while(1){
            // Skip the End Notes
            if(rec_data->event[event_offset] == 0x3FFF){
                event_offset++;
                break;
            }
            
            rec_file->p1_events[i].event[dest_slot] = rec_data->event[event_offset];
            dest_slot++;
            event_offset++;
        }
    }
    // Load Player 2 Events
    for(int i=0;i<8;i++){
        unsigned dest_slot = 0;
        while(1){
            // Skip the End Notes
            if(rec_data->event[event_offset] == 0x3FFF){
                event_offset++;
                break;
            }
            
            rec_file->p2_events[i].event[dest_slot] = rec_data->event[event_offset];
            dest_slot++;
            event_offset++;
        }
    }
    // Load Sound Events
    for(int i=0;i<8;i++){
        unsigned dest_slot = 0;
        while(1){
            // Skip the End Notes
            if(rec_data->event[event_offset] == 0x3FFF){
                event_offset++;
                break;
            }
            
            rec_file->sound_events[i].event[dest_slot] = rec_data->event[event_offset];
            dest_slot++;
            event_offset++;
        }
    }
    return 1;
}
