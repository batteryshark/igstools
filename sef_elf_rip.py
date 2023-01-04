# Percussion Master NoteChart ELF Ripper - 2023 rFx

"""
The PercussionMaster binary appears to contain a copy of notecharts embedded within the "band1" section. It's unknown if these represent the finished version of the notecharts, although their versions appear to match what's requested from the card itself.

The format is a little goofy and closely mirrors that which is in the rec files (dev recordings for making your own charts).

It should also be noted that the staff,howtoplay, and opening contain entries as well slightly before the songids start, however these may certainly be out of date based on the song length (the devs hardcoded the real song lengths into the game). It's possible these were for Rock Fever or an earlier revision.

While not every 'track' is figured out at present. This script, given the binary, will convert the embedded charts into the SEF format (the format the emulator uses to play the songs).

It should be noted that Rock Fever 4 appears to have this same structure. As a result, it's likely possible to reconstruct that game's songs as well using this method.
"""
import struct,os,sys

ELF_BASE = 0x08048000

BAND1_ROOT_ADDR = 0x0811D3E4
MAX_BAND_ADDR = 0x0811FE78
SONGINFO_SIZE = 0x120
BANDENTRY_SIZE = 100
"""
struct song_info{
unsigned short tempo; // this is bpm  as a bcd
unsigned short song_length; // length of the song in 32nd notes 
unsigned char stars; // difficulty ranking
unsigned char section_id;
unsigned char song_version; 
unsigned char idk; 
unsigned int  chart_difficulty; // from 0-8 (Rookie,Easy,Normal,Hard,Special,Challenge,Drum Master,None, Battle)
unsigned short num_notes;
unsigned short other_num_notes; // this looks like slightly less than num_notes?
unsigned int idk3; // idk, it's 1 on some.
unsigned short tempo_2; // idk maybe this is a new part of the structure.
unsigned char idk_somevals[78];
unsigned char* tracks[36]; // tracks cover cursors and sound cues it looks like.
unsigned char track_enabled[36]; // marked as 1 if the above track has stuff in it.
unsigned int idk_align[2];
};
"""

"""
typedef struct _band_entry{
void* info_1;
void* info_2;
const char* rom_path[84];
const char** wave_info;
unsigned int num_wave_files;
}BAND_ENTRY,*PBAND_ENTRY;
"""

def addr2raw(addr):
	return addr - ELF_BASE
	
def parse_songinfo(data):
    
    total_notes = struct.unpack("<H",data[12:14])[0]
    min_notes = struct.unpack("<H",data[14:16])[0]
    pass_percentage = min_notes / total_notes
    
    info = {
        'tempo':struct.unpack("<H",data[0:2])[0] / 100,
        'num_beats':struct.unpack("<H",data[2:4])[0],
        'pass_percentage':pass_percentage,
        'track_offsets':[],
        'sound_offsets':[],
        'soundevents':[]
    }
    # We have to find enabled tracks and add them now.
    for i in range(0,36):
        if(data[244 + i] == 1):
            # We're only capturing the first 6 tracks because they are the only ones used for notes
            if(i < 7):
                track_addr = struct.unpack("<I",data[100 + (4*i):(100 + (4*i)) + 4])[0]            
                info['track_offsets'].append(addr2raw(track_addr))
                continue
            # These two tracks are for sound events
            if(i == 16 or i == 17):
                track_addr = struct.unpack("<I",data[100 + (4*i):(100 + (4*i)) + 4])[0]
                info['sound_offsets'].append(addr2raw(track_addr))
                continue
                
            
            
    return info

def parse_noteinfo(val,track_num):
    
    note_info = {
        'lane': track_num,
        'beat': val & 0x3FFF,
        'holdflag':0,
        'exflag': 0,
        'swoff':0,
        'raw':val
    }
    return note_info
#   *(_DWORD *)&wTotalNote[2 * v7 + 2] = ((someval_from_buffer & 0x3FF) << 14) | bgnowevent_buffer & 0x3FFF | *(_DWORD *)&wTotalNote[2 * v7 + 2] & 0xFF000000;
def parse_soundevent(val):
    soundevent = {
        'beat': val & 0x3FFF,
        'value': (val >> 0x0E) + 1
    }
    
    return soundevent

# The notes with the flags don't count as one note... there are two to a pair, those values count as one 
# right shift 2 to get the fever count then take the first time minus the second to get the stretch value.

def parse_trackinfo(f,info):
    notes = []
    note_count = 0
    for track_num in range(0,len(info['track_offsets'])):        
        # Set our Offset 
        f.seek(info['track_offsets'][track_num])

        
        
        while 1:
            val = struct.unpack("<I",f.read(4))[0]    
            if val == 0x3FFF:
                break      
            # Determine if this is a hold note
            if(val & 0xFF000000 > 0 and ((val >> 0x18) & 1) > 0):               
                fever_count = (val & 0xFF000000) >> 0x1A
                note_count += fever_count
                next_val = struct.unpack("<I",f.read(4))[0]   
                val_diff = ((next_val & 0x3FFF) - (val & 0x3FFF)) * -14 # why -14? why not!
                
                note_info = {
                    'lane':track_num,
                    'beat':val & 0x3FFF,
                    'holdflag':(fever_count << 2) | 0x02,
                    'exflag': 0,
                    'swoff': val_diff
                }                              
                notes.append(note_info)
                continue
            
            # If not a hold note, we'll do other stuff
            if track_num in [2,4]:
                note_count+=2
            else:
                note_count+=1
            notes.append(parse_noteinfo(val,track_num))
        
    info['notes'] = notes
    del info['track_offsets']
    # Calculate total notes and min notes to pass based on passing percentage
    info['total_notes'] = note_count
    info['min_notes'] = int(info['total_notes'] * info['pass_percentage'])
    
    # Parse the sound event tracks.
    for entry in info['sound_offsets']:
        f.seek(entry,0)
        val = struct.unpack("<I",f.read(4))[0]
        # We break out of reading from this track if we hit the end marker.
        if(val == 0x3FFF):
            continue
        info['soundevents'].append(parse_soundevent(val))
        while val != 0x3FFF:
            val = struct.unpack("<I",f.read(4))[0]            
            if(val == 0x3FFF):
                break            
            info['soundevents'].append(parse_soundevent(val))
        
    del info['sound_offsets']
    
    return info

def generate_sef_data(info):
    sef_data = b""
    # Write Header 
    sef_data += struct.pack("<f",info['tempo'])
    sef_data += struct.pack("<I",info['num_beats'])
    sef_data += struct.pack("<I",info['total_notes'])
    sef_data += struct.pack("<I",info['min_notes'])
    sef_data += struct.pack("<I",len(info['soundevents']))
    sef_data += struct.pack("<I",len(info['notes']))
    
    # Write Sound Events
    for i in range(0,len(info['soundevents'])):
        sef_data += struct.pack("<h",info['soundevents'][i]['beat'])
        sef_data += struct.pack("<h",info['soundevents'][i]['value'])
    
    # Write Cursor Events
    for i in range(0,len(info['notes'])):
        cnote = info['notes'][i]
        sef_data += struct.pack("<h",cnote['beat'])
        # 2 bytes for the flags
        flag = (cnote['lane'] << 6) & 0xFFFF     
        flag |= 2
        sef_data += struct.pack("<H",flag)
        
        # 1 byte for our exflag - I don't think we use the measure flag
        sef_data += b"\x00"
        
        # 1 byte for our hold flag 
        hold_flag = 0
        swoff = 0
        # Note - this is bullshit for now because I don't understand this part.
        # Note 2: We're disabling the fever hold steps for now because they're fucked up.
        if(cnote['holdflag'] != 0):
            hold_flag = cnote['holdflag']
            swoff = cnote['swoff'] 
        
        sef_data += struct.pack("B",hold_flag)
        # 2 bytes for the swoff 
        sef_data += struct.pack("<h",swoff)    
    
    return sef_data


# First of all, let's get the MAX_BAND - that is, the number of charts in the elf.
f = open("peng","rb")

f.seek(addr2raw(MAX_BAND_ADDR),0)
max_band = struct.unpack("<I",f.read(4))[0]

print(f"This ELF contains {max_band} charts.")

song_charts = []
band_entries = []
f.seek(addr2raw(BAND1_ROOT_ADDR),0)
# First, read the root band entries. We only care about the offset to the chart data (the first value)
# Actually, we should probably also grab the number of wavs as that's the BG val too.
for i in range(0,max_band):
    entry_data = f.read(BANDENTRY_SIZE)
    chartdata_address = struct.unpack("<I",entry_data[0:4])[0]
    num_waves = struct.unpack("<I",entry_data[96:100])[0]
    band_entry = {
        'chart_address':addr2raw(chartdata_address),
        'num_waves':num_waves
    }
    band_entries.append(band_entry)
    

# Now that we know where all of the chart offsets are, we can parse those.    
for i in range(0,max_band):
    f.seek(band_entries[i]['chart_address'])
    chart_data = f.read(SONGINFO_SIZE)
    song_charts.append(parse_songinfo(chart_data))
    
# For each song, for each track offset, we have to parse 4 bytes at a time until we hit 0x3FFF which signals the end of the notes for that track, we'll need to pull these values apart and dump them into something useful.
for i in range(0,max_band):
    song_charts[i] = parse_trackinfo(f,song_charts[i])

# Now we can turn these into SEF files.

"""
    print(song_charts[i])
    print(len(song_charts[i]['notes']))
    exit(1)

"""

# Let's look at cha cha queen (3) to tune this thing.
"""
ccq = song_charts[3]
print(ccq)
exit(1)
"""

for i in range(0,max_band):
    sef_data = generate_sef_data(song_charts[i])
    with open("event/0_%d.sef" % i,"wb") as g:
        g.write(sef_data)
