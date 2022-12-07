// Song Table Data for PercussionMaster

#include <stdio.h>
#include <string.h>


struct SoundIndexEntry{
    unsigned short note;
    unsigned char index;
    unsigned char type;
    unsigned char id;    
};

struct NoteEntry{
    unsigned short note;
    unsigned char player;
    unsigned short index;        
    unsigned char data[8];
};

struct BGCueEntry{
    unsigned short note;
    unsigned char index;
    unsigned short value;
};

struct SongEntry{
    unsigned short song_id;
};



