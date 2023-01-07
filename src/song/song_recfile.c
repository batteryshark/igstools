// Handling Code for PM1 Recfile Reading
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "song_recfile.h"

#define RECFILE_MAGIC "PM1RECFILE  00.2004.0001"
// These are hardcoded so no worries on that.
#define OFFSET_REC_P1_CURSOR 0x196
#define OFFSET_REC_P2_CURSOR 0x7EC2
#define OFFSET_REC_SOUND     0xFBEE

int LoadRecfile(const char* path_to_recfile,PRecFile rec_file){
    FILE* fp = fopen(path_to_recfile,"rb");
    if(fp == NULL){
        printf("Error: Opening RecFile: %s\n",path_to_recfile);
        return 0;
    }
    
    // Read File Header Data and Check Magic
    fread(&rec_file->header,sizeof(rec_file->header),1,fp);    
    if(strcmp(rec_file->header.magic,RECFILE_MAGIC)){
        printf("Error: Invalid RecFile: %s\n",path_to_recfile);
        fclose(fp);
        return 0;
    }
    
    // Load the Player 1 Cursor Events    
    fseek(fp,OFFSET_REC_P1_CURSOR,SEEK_SET);
    fread(rec_file->p1_events,sizeof(RecFileLane),1,fp);
    
    // Load the Player 2 Cursor Events
    fseek(fp,OFFSET_REC_P2_CURSOR,SEEK_SET);
    fread(rec_file->p2_events,sizeof(RecFileLane),1,fp);
    
    // Load the Sound Events
    fseek(fp,OFFSET_REC_SOUND,SEEK_SET);
    fread(rec_file->sound_events,sizeof(RecFileLane),1,fp);        

    fclose(fp);

    return 1;
}
