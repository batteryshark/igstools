#ifndef __RECFILE_H
#define __RECFILE_H

typedef struct _RECFILE_EVENT_LANE{
    unsigned int num_events;
    unsigned int event[1000];
}RecFileLane,*PRecFileLane;


typedef struct _RECFILE_HEADER{
    char magic[24];
    unsigned int chart_id;
    unsigned short hbpm;
    unsigned short num_beats;
    unsigned char stars;
    unsigned char section_id;
    unsigned char song_name_id;
    unsigned char artist_name_id;
    unsigned short chart_difficulty_id;
    unsigned short idk_1;
    unsigned short total_notes;
    unsigned short min_notes;
    unsigned short idk_2;
    unsigned short idk_3;
    unsigned short hbpm_2;
    unsigned char  idk_4[78];    
}RecFileHeader,*PRecFileHeader;

typedef struct _RECFILE_DATA{    
    RecFileHeader header;
    RecFileLane p1_events[8];
    RecFileLane p2_events[8];
    RecFileLane sound_events[8];    
}RecFile,*PRecFile;

int LoadRecfile(const char* path_to_recfile,PRecFile rec_file);

#endif
