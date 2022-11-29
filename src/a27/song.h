#ifndef __SONG_H
#define __SONG_H

#include "a27.h"

void Song_UploadPlaybackHeader(const unsigned char* in_data, struct A27_Read_Message* msg);
void Song_UploadPlaybackBody(const unsigned char* in_data, struct A27_Read_Message* msg);
void Song_MainGameSetting(const unsigned char* in_data, struct A27_Read_Message* msg);
void Song_MainGameWaitStart(const unsigned char* in_data, struct A27_Read_Message* msg);
void Song_MainGameStart(const unsigned char* in_data, struct A27_Read_Message* msg);
void Song_MainGameProcess(const unsigned char* in_data, struct A27_Read_Message* msg);
void Song_ResultProcess(const unsigned char* in_data, struct A27_Read_Message* msg);

#endif
