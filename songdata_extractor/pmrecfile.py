# Script to Generate a PMREC File for PercussionMaster
import os,sys,struct,binascii

# the header is validated by the game and the song info is useful
def generate_header(band_info):
    header_data = b"PM1RECFILE  00.2004.0001"
    song_info = band_info['song_info']
    header_data += struct.pack("<I",band_info['band_id'])
    header_data += struct.pack("<H",song_info['tempo'])
    header_data += struct.pack("<H",song_info['song_length'])
    header_data += struct.pack("B",song_info['stars'])
    header_data += struct.pack("B",song_info['section_id'])
    header_data += struct.pack("B",song_info['song_name_id'])
    header_data += struct.pack("B",song_info['artist_name_id'])
    header_data += struct.pack("<H",song_info['rank_id'])
    header_data += struct.pack("<H",song_info['total_beats'])
    header_data += struct.pack("<H",song_info['total_cursors'])
    header_data += struct.pack("<I",song_info['idk'])
    header_data += struct.pack("<H",song_info['tempo_2'])
    header_data += song_info['idk_2']
    return header_data
    
# this isn't used so we don't care
def generate_wavedir_section():
    return b"WAVEDIR "+struct.pack("<I",256)+(b"\x00" * 256)
    
# section titles can be TREC1,TREC2,TRECBG and contain 4 byte event values    
def generate_event_section(section_title,event_db):
    event_data = bytes(section_title)
    event_data += struct.pack("<I",0x7D20)
    eb = bytearray(0x7D20)
    for i in range(0,len(event_db)):
        track_offset = 4004 * i
        eb[track_offset:track_offset+4] = struct.pack("<I",len(event_db[i]))
        for j in range(0,len(event_db[i])):
            ce = event_db[i][j]
            entry_offset = track_offset+4+(j*4)
            eb[entry_offset:entry_offset+4] = struct.pack("<I",ce)
    event_data += eb
    return event_data
    
def generate_footer():
    return b" FILEEND\x00\x00\x00\x00"


def generate_rec_file(band_info):
    header = generate_header(band_info)
    wavedir = generate_wavedir_section()
    p1_section = generate_event_section(b"TREC1   ",band_info['song_info']['cursor_events'])
    p2_section = generate_event_section(b"TREC2   ",band_info['song_info']['cursor_events'])
    bg_section = generate_event_section(b"TRECBG  ",band_info['song_info']['sound_events'])
    footer = generate_footer()
    return header+wavedir+p1_section+p2_section+bg_section+footer
        
    