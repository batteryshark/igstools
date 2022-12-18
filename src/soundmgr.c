// FMOD Replacement Engine - Big Thanks to ChatGPT for the Assist!
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fmodex/fmod.h>
#define MAX_CHANNELS 100
FMOD_SYSTEM *fmodsystem;
#define SOUND_DEBUG 1
#include <sys/time.h>
  

typedef struct _SOUNDMGR_ENTRY{
    char* path_to_soundfile;
    FMOD_SOUND* sound;
    FMOD_CHANNEL* channel;
}SoundEntry,*PSoundEntry;

static SoundEntry soundtable[MAX_CHANNELS];
unsigned char credit_sound_loaded = 0;

void SoundMgr_Init(void){
#ifdef SOUND_DEBUG
    printf("[SoundMgr::Init]\n");
#endif 
  // Initialize FMOD
  FMOD_RESULT result;
  result = FMOD_System_Create(&fmodsystem);
  if(result){printf("FMOD_System_Create Error!\n");exit(-1);}
  result = FMOD_System_Init(fmodsystem, MAX_CHANNELS, FMOD_INIT_NORMAL, 0);
    if(result){printf("FMOD_System_Init Error!\n");exit(-1);}
FMOD_System_SetDSPBufferSize(fmodsystem,8,1);
}

void SoundMgr_Shutdown(void){
    #ifdef SOUND_DEBUG
    printf("[SoundMgr::Shutdown]\n");
#endif 
  FMOD_System_Close(fmodsystem);
  FMOD_System_Release(fmodsystem);
}

void SoundMgr_LoadSlot(const char* path_to_soundfile, int slot,char should_loop){
    FMOD_RESULT result;    
    result = FMOD_System_CreateSound(fmodsystem, path_to_soundfile, FMOD_CREATESAMPLE, 0, &soundtable[slot].sound);
    if(result){printf("FMOD createSound Error!\n");exit(-1);}
    soundtable[slot].path_to_soundfile = malloc(strlen(path_to_soundfile)+1);
    strcpy(soundtable[slot].path_to_soundfile,path_to_soundfile);   
    if(should_loop){FMOD_Sound_SetMode(soundtable[slot].sound,FMOD_LOOP_NORMAL);   
    }
    FMOD_System_PlaySound(fmodsystem, soundtable[slot].sound, 0, 1, &soundtable[slot].channel);
}

// Load the WAV file into a sound object
int SoundMgr_Load(const char* path_to_soundfile, int should_loop){
    #ifdef SOUND_DEBUG
    printf("[SoundMgr::Load] %s %d\n",path_to_soundfile,should_loop);
#endif 
  int slot = -1;
  for(slot = 0; slot < MAX_CHANNELS; slot++){
        if(soundtable[slot].sound == NULL){
            SoundMgr_LoadSlot(path_to_soundfile,slot,should_loop);
            return slot;
        }
  }

    printf("[SoundMgr::Load] Could not Find a Free Sound Slot!\n");
    exit(-1);
    return -1;
}



void SoundMgr_Stop(unsigned int slot){
 #ifdef SOUND_DEBUG
    printf("[SoundMgr::Stop] %d\n",slot);
#endif 
    FMOD_Channel_Stop(soundtable[slot].channel); 
}

void SoundMgr_Unload(int slot){
    // We won't unload the credit sound.
    if(slot == (MAX_CHANNELS-1)){return;}
    #ifdef SOUND_DEBUG
    printf("[SoundMgr::Unload] %d\n",slot);
#endif 
    if(soundtable[slot].sound != NULL){
    // If the sound is playing, we need to stop it first.
    FMOD_BOOL is_playing;
    FMOD_Channel_IsPlaying(soundtable[slot].channel, &is_playing);
    if(is_playing){
        SoundMgr_Stop(slot);
    }
     free(soundtable[slot].path_to_soundfile);
     soundtable[slot].path_to_soundfile = NULL;
     FMOD_Sound_Release(soundtable[slot].sound);
     soundtable[slot].sound = NULL;
    }
}

void SoundMgr_LoadCredit(const char* path_to_soundfile){
    #ifdef SOUND_DEBUG
    printf("[SoundMgr::LoadSimple] %s\n",path_to_soundfile);
#endif 
    if(!credit_sound_loaded){
        SoundMgr_LoadSlot(path_to_soundfile,(MAX_CHANNELS-1),0);
        credit_sound_loaded = 1;
    }
}

void SoundMgr_UnloadAll(void){
    #ifdef SOUND_DEBUG
   // printf("[SoundMgr::UnloadAll]\n");
#endif 
 for(int slot = 0; slot < MAX_CHANNELS; slot++){
     SoundMgr_Unload(slot);
 }
}

void SoundMgr_Wait(int slot){    
    #ifdef SOUND_DEBUG
    printf("[SoundMgr::Wait] %d\n",slot);
#endif 
    if(slot >= MAX_CHANNELS){
        printf("Error - slot >= %d\n",MAX_CHANNELS);
        return;
    }
    FMOD_BOOL isPlaying = 1;
    while (isPlaying) {
        FMOD_Channel_IsPlaying(soundtable[slot].channel, &isPlaying);
    }   
}


void SoundMgr_Play(unsigned int slot, float volume){

    #ifdef SOUND_DEBUG
    //printf("[SoundMgr::Play] %d %f\n",slot,volume);
#endif 
    if(slot >= MAX_CHANNELS){
     printf("Error - Slot >= %d\n",MAX_CHANNELS);
     return;
    }
    FMOD_Channel_SetPaused(soundtable[slot].channel,0);
}

void SoundMgr_PlayCredit(float volume){
    SoundMgr_Stop((MAX_CHANNELS-1));
    SoundMgr_Play((MAX_CHANNELS-1),volume);
}

void SoundMgr_Update(void){
    FMOD_System_Update(fmodsystem);   
}


