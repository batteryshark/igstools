#include <stdio.h>
#include <string.h>


#include "../utils.h"
#include "song_recfile.h"
#include "song_event.h"

// Pixel-Per-Beat, used in calculating Fever Beats and To a Lesser Extent Scrolling.
// First Value is Normal, Followed by HiSpeed 1,2,3,4. Any BPM over 145 is high speed.
static const unsigned char cursor_ppb_fast[5] = {12,18,20,24,27};
static const unsigned char cursor_ppb_slow[5] = {14,20,24,27,32};

unsigned char calculate_velocity(float tempo, unsigned int speed_mod){
        if(tempo > 145.0f){
            return cursor_ppb_fast[speed_mod];
        }
        
        return cursor_ppb_slow[speed_mod];
}


unsigned int ParseCursorEvents(PRecFileLane* plane,PCursorEvent* cursor_events, float tempo, unsigned char speed_mod){
    unsigned char song_velocity = calculate_velocity(tempo,0);
    unsigned char player_velocity = calculate_velocity(tempo,speed_mod);
    unsigned int num_cursor_events = 0;
    for(int i=0;i<8;i++){
        PRecFileLane current_lane = plane[i];
        for(int j=0;j<current_lane->num_events;j++){
            // Standard Values we need.            
            cursor_events[num_cursor_events]->event_beat = current_lane->event[j] & 0x3FFF;
            cursor_events[num_cursor_events]->spawn_beat = (current_lane->event[j] & 0x3FFF) - song_velocity;
            cursor_events[num_cursor_events]->flags = (i << 6) | 0x02;
            if(((current_lane->event[j] >> 0x18) & 0x01) > 0){
                unsigned char fever_count = ((current_lane->event[j] & 0xFF000000) >> 0x1A) & 0xFF;
                cursor_events[num_cursor_events]->fever_flag = (fever_count << 2) | 0x02;
                short fever_distance = (current_lane->event[j+1] & 0x3FFF) - (current_lane->event[j] & 0x3FFF);
                fever_distance *= -1;
                // Calculate Pixels Per Beat                
                cursor_events[num_cursor_events]->fever_offset = fever_distance * player_velocity;
                // Skip ahead due to multi value entry
                j++;
            }            
            num_cursor_events++;            
        }        
    }
    return num_cursor_events;
}

unsigned int ParseSoundEvents(PRecFileLane* plane, PSoundEvent* sound_events, float tempo){
    unsigned char song_velocity = calculate_velocity(tempo,0);
    unsigned int num_sound_events = 0;
    for(int i=0;i<8;i++){
        PRecFileLane current_lane = plane[i];
        for(int j=0;j<current_lane->num_events;j++){
             // Standard Values we need.
            sound_events[num_sound_events]->event_beat = current_lane->event[j] & 0x3FFF;
            sound_events[num_sound_events]->spawn_beat = (current_lane->event[j] & 0x3FFF) - song_velocity;
            sound_events[num_sound_events]->event_value = (current_lane->event[j] >> 0x0E) + 1;
            num_sound_events++;            
        }
        
    }
    return num_sound_events;
}

void ParseRecHeader(PRecFile rec_file, PSongEvent song_event){
    song_event->tempo = short_to_float(rec_file->header.hbpm);
    song_event->chart_id = rec_file->header.chart_id;
    song_event->num_beats = rec_file->header.num_beats;
    song_event->total_notes = rec_file->header.total_notes;
    song_event->min_notes = rec_file->header.min_notes;
    song_event->scroll_velocity = calculate_velocity(song_event->tempo,0);
}



