#include <stdio.h>
#include <string.h>


#include "../../utils/utils.h"
#include "song_recfile.h"
#include "song_event.h"

// Pixel-Per-Beat, used in calculating Fever Beats and To a Lesser Extent Scrolling.
// First Value is Normal, Followed by HiSpeed 1,2,3,4. Any BPM over 145 is high speed.
static const unsigned char cursor_ppb_fast[5] = {12,18,20,24,27};
static const unsigned char cursor_ppb_slow[5] = {14,20,24,27,32};
unsigned int speed_mod_spawn_ms[5] = {24,20,16,12,8};

unsigned char calculate_velocity(float tempo, unsigned int speed_mod){
        if(tempo > 145.0f){
            return cursor_ppb_fast[speed_mod];
        }
        
        return cursor_ppb_slow[speed_mod];
}

void GenerateMeasureCursors(PSongEvent event, unsigned char speed_mod_p1, unsigned char speed_mod_p2){
    float ms_per_beat = ((float)((60 * 1000) / event->tempo)) / 8;   
    event->num_measure_events = (unsigned int)((float)event->num_beats/32) + 1;
    for(int i=0;i<event->num_measure_events;i++){
            event->measure_events[0].measure[i].event_ms = (i * event->ms_per_measure);
            event->measure_events[0].measure[i].spawn_ms = event->measure_events[0].measure[i].event_ms - (ms_per_beat * speed_mod_spawn_ms[speed_mod_p1]);
            event->measure_events[1].measure[i].event_ms = (i * event->ms_per_measure);
            event->measure_events[1].measure[i].spawn_ms = event->measure_events[1].measure[i].event_ms - (ms_per_beat * speed_mod_spawn_ms[speed_mod_p2]);            
    }
}


unsigned int ParseCursorEvents(PRecFileLane* plane,PCursorEvent* cursor_events, float tempo, unsigned char speed_mod){
    float ms_per_beat = ((float)((60 * 1000) / tempo)) / 8;   
    
    unsigned char player_velocity = calculate_velocity(tempo,speed_mod);
    unsigned int num_cursor_events = 0;
    for(int i=0;i<8;i++){
        PRecFileLane current_lane = (PRecFileLane)plane+i;
        for(int j=0;j<current_lane->num_events;j++){
            // Standard Values we need.            
            PCursorEvent cse = (PCursorEvent)cursor_events+num_cursor_events;
            cse->event_ms = (long)((float)(current_lane->event[j] & 0x3FFF) * ms_per_beat);
            cse->spawn_ms = cse->event_ms - (ms_per_beat * speed_mod_spawn_ms[speed_mod]);
            cse->flags = (i << 6) | 0x02;
            if(((current_lane->event[j] >> 0x18) & 0x01) > 0){
                unsigned char fever_count = ((current_lane->event[j] & 0xFF000000) >> 0x1A) & 0xFF;
                cse->fever_flag = (fever_count << 2) | 0x02;
                short fever_distance = (current_lane->event[j+1] & 0x3FFF) - (current_lane->event[j] & 0x3FFF);
                fever_distance *= -1;
                // Calculate Pixels Per Beat                
                cse->fever_offset = fever_distance * player_velocity;
                // Skip ahead due to multi value entry
                j++;
            }            
            num_cursor_events++;            
        }        
    }
    return num_cursor_events;
}

unsigned int ParseSoundEvents(PRecFileLane* plane, PSoundEvent* sound_events, float tempo){
    float ms_per_beat = ((float)((60 * 1000) / tempo)) / 8;   
    unsigned int num_sound_events = 0;
    for(int i=0;i<8;i++){
        PRecFileLane current_lane = (PRecFileLane)plane+i;
        for(int j=0;j<current_lane->num_events;j++){
             // Standard Values we need.
            PSoundEvent cse = (PSoundEvent)sound_events+num_sound_events;
            cse->event_ms = (long)((float)(current_lane->event[j] & 0x3FFF) * ms_per_beat);
            // TODO: This feels roughly right - mess with this multiplier to sync the music with the beats.
            cse->spawn_ms = cse->event_ms - (ms_per_beat * 8);   
            cse->event_value = (current_lane->event[j] >> 0x0E) + 1;
            num_sound_events++;            
        }
        
    }
    return num_sound_events;
}

void ParseRecHeader(PRecFile rec_file, PSongEvent song_event){
    song_event->tempo = Short2Float(rec_file->header.hbpm);
    // Given Beats Per Minute, calculate how many ms per beat. (quarter note)
    song_event->ms_per_beat = (float)((60 * 1000) / song_event->tempo);
    song_event->ms_per_measure = song_event->ms_per_beat * 4;
    song_event->ms_per_ebeat = (song_event->ms_per_beat / 8);
    printf("Tempo: %.2f MSPerBeat: %.2f MSPerEBeat: %.2f MSPerMeasure: %.2f\n",song_event->tempo, song_event->ms_per_beat, song_event->ms_per_ebeat, song_event->ms_per_measure);
    song_event->chart_id = rec_file->header.chart_id;
    song_event->num_beats = rec_file->header.num_beats;
    song_event->total_notes = rec_file->header.total_notes;
    song_event->min_notes = rec_file->header.min_notes;
    song_event->scroll_velocity = calculate_velocity(song_event->tempo,0);
}

void SetPlayerVelocity(PSongEvent song_event, unsigned char p1_speed_mod, unsigned char p2_speed_mod){
    song_event->player_velocity[0] = calculate_velocity(song_event->tempo,p1_speed_mod);   
    song_event->player_velocity[1] = calculate_velocity(song_event->tempo,p2_speed_mod);
}
