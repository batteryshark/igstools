# Recfile Ripper for PercussionMaster Game
import os,sys,struct,binascii
from pmrecfile import generate_rec_file

ELF_BASE = 0x08048000

g_pchaSongName = 0x08171CE0
m_pchaManName = 0x08171D60 
g_pchaSongRank = 0x08171D94
g_iaMaxBand = 0x0811FE78
g_raBand1List = 0x0811D3E4


def addr2raw(addr,section_offset=0):
    return addr - (ELF_BASE + section_offset)

def read_cstring_from_offset(pb,offset):
    pb.seek(offset,0)
    return ''.join(iter(lambda: pb.read(1).decode('ascii'), '\x00'))    

def read_string_from_ptrtable(pb,table_address,table_id):
    # Get the ptr address to our song name string.
    ptroffset = addr2raw(table_address + (table_id * 4),0x1000)
    pb.seek(ptroffset,0)
    straddr = struct.unpack("<I",pb.read(4))[0]
    # Get the string from our address (null terminated)
    return read_cstring_from_offset(pb,addr2raw(straddr))

def get_max_number_of_charts(pb):
    pb.seek(addr2raw(g_iaMaxBand),0)
    return struct.unpack("<I",pb.read(4))[0]

def get_song_info(pb,addr):
    pb.seek(addr2raw(addr))
    tempo = struct.unpack("<H",pb.read(2))[0]
    song_length = struct.unpack("<H",pb.read(2))[0]
    stars = struct.unpack("B",pb.read(1))[0]
    section_id = struct.unpack("B",pb.read(1))[0]
    song_name_id = struct.unpack("B",pb.read(1))[0]
    artist_name_id = struct.unpack("B",pb.read(1))[0]
    rank_id = struct.unpack("<H",pb.read(2))[0]
    idk_id = struct.unpack("<H",pb.read(2))[0]
    total_beats = struct.unpack("<H",pb.read(2))[0]
    total_cursors = struct.unpack("<H",pb.read(2))[0]
    idk = struct.unpack("<I",pb.read(4))[0]
    tempo_2 = struct.unpack("<H",pb.read(2))[0]
    idk_2 = pb.read(78)
    track_addr = []
    for i in range(0,36):
        track_addr.append(struct.unpack("<I",pb.read(4))[0])
    
    # We need the cursor tracks (0-7) and sound (16)
    cursor_events = {}
    for i in range(0,7):
        cursor_events[i] = []
        pb.seek(addr2raw(track_addr[i]))
        while 1:
            val = struct.unpack("<I",pb.read(4))[0]
            if val == 0x3FFF:
                break
            cursor_events[i].append(val)
            
    sound_events = {0:[]}
    pb.seek(addr2raw(track_addr[16]))
    while 1:
        val = struct.unpack("<I",pb.read(4))[0]
        if val == 0x3FFF:
            break
        sound_events[0].append(val)

    return {
        'tempo':tempo,
        'song_length':song_length,
        'stars':stars,
        'section_id':section_id,
        'song_name_id':song_name_id,
        'artist_name_id':artist_name_id,
        'rank_id':rank_id,
        'idk_id':idk_id,
        'total_beats':total_beats,
        'total_cursors':total_cursors,
        'idk':idk,
        'tempo_2':tempo_2,
        'idk_2':idk_2,
        'cursor_events':cursor_events,
        'sound_events':sound_events
    }

def get_band_entries(pb):
    band_entries = []
    number_of_entries = get_max_number_of_charts(pb)
    pb.seek(addr2raw(g_raBand1List),0)
    for i in range(0,number_of_entries):
        info = struct.unpack("<I",pb.read(4))[0]
        info_2 = struct.unpack("<I",pb.read(4))[0]
        rom_path = pb.read(84).replace(b"\x00",b"").decode('big5')
        wave_filepath_table_addr = struct.unpack("<I",pb.read(4))[0]
        num_wave_files = struct.unpack("<I",pb.read(4))[0]
        # Save our offset so we can come back to it on the next iteration
        saved_offset = pb.tell()
        wave_filepaths = []        
        for j in range(0,num_wave_files):
            wave_filepaths.append(read_string_from_ptrtable(pb,wave_filepath_table_addr,j))
        
        song_info = get_song_info(pb,info)
        entry = {
            'band_id':i,
            'song_info':song_info,
            'rom_path':rom_path,
            'wave_files':wave_filepaths
        }
        band_entries.append(entry)
        pb.seek(saved_offset)
    return band_entries


def read_song_name(pb,song_id):
    return read_string_from_ptrtable(pb,g_pchaSongName,song_id)

def read_artist_name(pb,song_id):
    return read_string_from_ptrtable(pb,m_pchaManName,song_id)

def read_rank_name(pb,song_id):
    return read_string_from_ptrtable(pb,g_pchaSongRank,song_id)
    
def generate_recfile_name(pb,song_id,rank_id):
    return read_song_name(pb,song_id)+"_"+read_rank_name(pb,rank_id)+".rec"

def create_recfile(pb,entry):
    recfile_name = generate_recfile_name(pb,entry['song_info']['song_name_id'],entry['song_info']['rank_id'])
    print("Creating Recfile: %s..." % recfile_name)
    recfile_path = os.path.join("songdata",recfile_name)
    if not os.path.exists("songdata"):
        os.makedirs("songdata")
    with open(recfile_path,"wb") as g:
        g.write(generate_rec_file(entry))
    
    

if __name__ == "__main__":
    pb = open("peng","rb")    
    be = get_band_entries(pb)
    for entry in be:
        create_recfile(pb,entry)
    print("DonionRingz!")
