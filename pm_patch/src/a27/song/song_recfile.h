#ifndef __RECFILE_H
#define __RECFILE_H

enum PM_Song_Mode{
    SONG_MODE_NORMAL,
    SONG_MODE_DEMO,
    SONG_MODE_OPENING,
    SONG_MODE_STAFF,
    SONG_MODE_HOW_TO_PLAY
};

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
    unsigned short total_notes;
    unsigned short min_notes;
    unsigned char p1_enable;
    unsigned char p2_enable;
    unsigned short idk_2;
    unsigned short hbpm_2;
    unsigned char  idk_3[78];
}RecFileHeader,*PRecFileHeader;

typedef struct _RECFILE_DATA{    
    RecFileHeader header;
    RecFileLane p1_events[8];
    RecFileLane p2_events[8];
    RecFileLane sound_events[8];    
}RecFile,*PRecFile;

typedef struct _A27_RECDATA_HEADER{
	unsigned short cmd;
	unsigned short hbpm;
	unsigned short num_beats;
	unsigned char stars;
	unsigned char section_id;
	unsigned char song_name_id;
	unsigned char artist_name_id;
	unsigned short chart_difficulty_id;
	unsigned short total_notes;
	unsigned short min_notes;
	unsigned short idk_1;
	unsigned char p1_enable;
	unsigned char p2_enable;
	unsigned short idk_2;
	unsigned short hbpm_2;
	unsigned char idk_3[78];
	// Remember that the totals in this case include the 0x3FFF 'end' marker too.
	unsigned short p1_track_total_events[8];
	unsigned short p2_track_total_events[8];
	unsigned short sound_track_total_events[8];
}RecDataHeader,*PRecDataHeader;

typedef struct _A27_RECDATA_BODY{
	unsigned short cmd;
	unsigned short idk;
	unsigned int event[1000];
}A27RecDataBody,*PA27RecDataBody;

int LoadRecfile(const char* path_to_recfile,PRecFile rec_file);
int LoadRecHeader(PRecDataHeader rec_header_data, PRecFile rec_file);
int LoadRecData(PA27RecDataBody rec_data, PRecFile rec_file);
int GenerateEventFilename(unsigned int song_mode, unsigned int chart_id, char* event_file_path);
#endif
