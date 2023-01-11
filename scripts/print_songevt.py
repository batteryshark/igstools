# PercussionMaster Song Event Log
import binascii,struct,os,sys,json
from pm_sdb import *


         
def get_song_info(data):
    cmd = struct.unpack("<H",data[0:2])[0]
    state = struct.unpack("<H",data[2:4])[0]
    stage_num = data[3]
    game_mode = data[4]
    kr = data[5]
    p1_enable = data[6]
    p2_enable = data[7]
    p1_autoplay = data[8]
    p2_autoplay = data[9]
    p1_songversion = data[10]
    p2_songversion = data[11]  
    p1_songid = struct.unpack("<H",data[12:14])[0]    
    p2_songid = struct.unpack("<H",data[14:16])[0]
    song_version = p1_songversion
    if song_version == 0:
        song_version = p2_songversion
    song_id = p1_songid
    if song_id == 0:
        song_id = p2_songid
    p1_speed =  data[16] 
    p1_cloak =  data[17]
    p1_noteskin =  data[18] 
    p1_align =  data[19] 
    p2_speed =  data[20]
    p2_cloak =  data[21]
    p2_noteskin =  data[22]
    p2_align =  data[23]
    judge_great = struct.unpack("<H",data[24:26])[0]
    judge_cool =  struct.unpack("<H",data[26:28])[0]
    judge_nice =  struct.unpack("<H",data[28:30])[0]
    judge_poor =  struct.unpack("<H",data[30:32])[0]
    align_3 = struct.unpack("<H",data[32:34])[0]
    p1_rating = data[34]
    p2_rating = data[35]
    level_rate_p1 = [
    struct.unpack("f",data[36:40])[0],
    struct.unpack("f",data[40:44])[0],
    struct.unpack("f",data[44:48])[0],
    struct.unpack("f",data[48:52])[0],
    struct.unpack("f",data[52:56])[0],
    struct.unpack("f",data[56:60])[0]    
    ]
    level_rate_p2  = [
    struct.unpack("f",data[60:64])[0],
    struct.unpack("f",data[64:68])[0],
    struct.unpack("f",data[68:72])[0],
    struct.unpack("f",data[72:76])[0],
    struct.unpack("f",data[76:80])[0],
    struct.unpack("f",data[80:84])[0]    
    ]
    idk_1 = binascii.hexlify(data[84:92]).decode('ascii')
    idk_p1 = data[92]
    idk_p2 = data[93]
    non_challenge_mode = data[94]
    song_mode = data[95]
    
    song_info = {
    'Stage':stage_num,
    'Game Mode':game_mode,
    'Key Record':kr,
    'P1 Enable':p1_enable,
    'P2 Enable':p2_enable,
    'P1 Autoplay':p1_autoplay,
    'P2 Autoplay':p2_autoplay,
    'P1 SongVersion':p1_songversion,
    'P2 SongVersion':p2_songversion,
    'P1 SongID':p1_songid,
    'P2 SongID':p2_songid,
    'Song Version':song_version,
    'Song ID': song_id,
    'P1 Speed':p1_speed,
    'P1 Cloak':p1_cloak,
    'P1 Noteskin':p1_noteskin,
    'P1 Align':p1_align,
    'P2 Speed':p2_speed,
    'P2 Cloak':p2_cloak,
    'P2 Noteskin':p2_noteskin,
    'P2 Align':p2_align,
    'Judge Great':judge_great,
    'Judge Cool':judge_cool,
    'Judge Nice':judge_nice,
    'Judge Poor':judge_poor,
    'Align 3':align_3,
    'P1 Rating':p1_rating,
    'P2 Rating':p2_rating,
    'Level Rate P1':level_rate_p1,
    'Level Rate P2':level_rate_p2,
    'IDK 1':idk_1,
    'IDK P1':idk_p1,
    'IDK P2':idk_p2,
    'Non Challenge Mode':non_challenge_mode,
    'Song Mode':song_mode
    }
    if(song_mode == 0):
        song_info['extended'] = GetMoreSongInfo(song_id)
    else:
        song_info['extended'] = GetNonSongInfo(song_mode)
        
    return song_info
    
def get_song_packets_from_dump(dump_path):
    packets = []
    f = open(dump_path,"rb")

    last_wdata = ""
    while(1):
        wheader = ""
        wseek_offset = f.read(4)
        if(wseek_offset == None):
            break
        wcmd = f.read(4)
        try:
            ts_w = struct.unpack("<Q",f.read(8))[0]
        except:
            break
        wdata_sz = struct.unpack("<I",f.read(4))[0]
        if(wdata_sz != 0):
            wheader = f.read(36)
            wdata = f.read(wdata_sz-36)
        else:
            wdata = ""
        rseek_offset = f.read(4)
        rcmd = f.read(4)
        ts_r_data = ""
        try:
            ts_r_data = f.read(8)
            ts_r = struct.unpack("<Q",ts_r_data)[0]
        except:
            break
        rdata_sz = struct.unpack("<I",f.read(4))[0]
        rhdr = f.read(132)
        rdata = f.read(rdata_sz-132)

        if(len(wdata) or len(rdata)):
            if(struct.unpack("<I",wheader[4:8])[0] == 15):
                packets.append([wdata,rdata,ts_r_data])
    return packets
    
    
def carve_songs_from_song_packets(packets):
    songs = {}
    song_start = 0
    song_end = 0    
    song_packets = []
    song_token = ""
    song_info = {}
    ctr = 0
    for packet in packets:
        req = packet[0]
        res = packet[1]
        ts_r_data = packet[2]
        if(req.startswith(b"\x05\x00")):
            # Set Song Timestamp Start
            song_start = struct.unpack("<Q",ts_r_data)[0]
            song_info = get_song_info(req)
            if song_info == None:
                print("Error - Why is song token none?!?")
                exit(1)
            # Make a Unique ID for Our Songs
            ctr +=1
            song_token = "%d_%d_%d_%d" % (song_info['Song Mode'],song_info['Song ID'],song_info['Song Version'],ctr)            
            song_packets = [(0,res)]
        if(req.startswith(b"\x06\x00")):
            # Get Delta from Start Time 
            ctime = struct.unpack("<Q",ts_r_data)[0]
            
            song_packets.append((ctime-song_start,res))
        if(req.startswith(b"\x09\x00")):
            # Get Delta from Start Time 
            ctime = struct.unpack("<Q",ts_r_data)[0]
            song_end = ctime
            
            if song_token != None:
                songs[song_token] = {
                'info':song_info,
                'packets':song_packets,
                'elapsed':song_end - song_start
                }
                song_token = None
                song_info = None
                song_packets = []
        
    return songs



ANI_SLOT_DB = {
0:"Blue",
1:"Drum",
2:"2Drum",
3:"Rim",
4:"2Rim",
5:"Red",
6:"Placeholder1",
7:"Placeholder2"
}

ANI_JUDGE_DB = {
1:"GREAT",
2:"COOL",
3:"NICE",
4:"POOR",
5:"LOST",
6:"BRAVO",
7:"IDK1",
8:"IDK2"
}

def derive_cursor_state(data):

    cursor_type = struct.unpack("<H",data[0:2])[0]
    if(cursor_type == 0):
        return None
    visible = (cursor_type & 2) != 0
    cursor_lane = (cursor_type >> 6) & 0xFF
    ext_flag = data[2]
    if ext_flag == 0x40:
        fct = "Measure Bar"
    elif ext_flag != 0:
        fct = "Unknown"
    else:
        fct = "Note: %s" % ANI_SLOT_DB[cursor_lane]
    
    hold_flag = data[3]
    if (hold_flag & 2) != 0:
        hold_val = hold_flag >> 2
        fct += " [Rapid Beat %d]" % hold_val
    
    y_pos = struct.unpack("<h",data[4:6])[0]
    ypf = "YPos: %d" % y_pos
    y_stretch = struct.unpack("<h",data[6:8])[0]
    if(y_stretch != 0):
        ypf += " [Stretch %d]" % y_stretch
        
    entry = {
    'type':fct,
    'pos': ypf,
    'vis': visible,
    'raw': binascii.hexlify(data)
    }
    return entry
    
    
    
def get_packet_event_info(packet):
    p1_note_counter = struct.unpack("<h",packet[4:6])[0]
    p2_note_counter = struct.unpack("<h",packet[6:8])[0]
    note_counter = p1_note_counter
    if note_counter == 0:
        note_counter = p2_note_counter
    sound_index = {}
    for i in range(0,64,2):
        cs = struct.unpack("<H",packet[8+i:8+2+i])[0]
        if cs != 0:
            sound_index[int(i/2)] = cs

    events = {
    'Note': note_counter
    }
    
    if sound_index != {}:
        events['SoundEvents'] = sound_index
    
    
    # Cursor Events
    cursor_status = {}
    p1_cursor = {}
    notechart_p1 = packet[0x48:0x4F8]
    for i in range(0,1200,8):
        slot = int(i/8)
        
        cs = derive_cursor_state(notechart_p1[i:i+8])
        if cs == None:
            continue
        p1_cursor[slot] = cs
        
    if p1_cursor != {}:
        cursor_status['p1_cursor'] = p1_cursor
    
    p2_cursor = {}
    notechart_p2 = packet[0x4F8:0x9A8]
    for i in range(0,1200,8):
        slot = int(i/8)
        
        cs = derive_cursor_state(notechart_p2[i:i+8])
        if cs == None:
            continue
        p2_cursor[slot] = cs
            
    if p2_cursor != {}:
        cursor_status['p2_cursor'] = p2_cursor   

    if cursor_status != {}:
        events['Cursor'] = cursor_status

    # Player Status Events
    
    player_status = {}
    
    p1_combo = struct.unpack("<H",packet[0x9A8:0x9AA])[0]
    if(p1_combo):
        player_status['p1_combo'] = p1_combo
        
    p2_combo = struct.unpack("<H",packet[0x9AA:0x9AC])[0]
    if(p2_combo):
        player_status['p2_combo'] = p2_combo
        
 
    player_status['p1_feverbeat'] = struct.unpack("<H",packet[0x9AC:0x9AE])[0]
        
    player_status['p2_feverbeat'] = struct.unpack("<H",packet[0x9AE:0x9B0])[0]
   
    
    p1_playing = packet[0x9B0]
    if(p1_playing):
        player_status['p1_play'] = p1_playing
        
    p2_playing = packet[0x9B1]
    if(p2_playing):
        player_status['p2_play'] = p2_playing    
    
    player_status['p1_holdcombo'] = packet[0x9B2]
    player_status['p2_holdcombo'] = packet[0x9B3]
    
    
    p1_judge_ani = {}
    
    for j in range(0,8):
        cv = packet[0x9B4+j]
        if(cv != 0):
            p1_judge_ani[ANI_SLOT_DB[j]] = ANI_JUDGE_DB[cv]

    if p1_judge_ani != {}:
        player_status['p1_judge_ani'] = p1_judge_ani

    p2_judge_ani = {}
    for j in range(0,8):
        cv = packet[0x9BC+j]
        if(cv != 0):
            p2_judge_ani[ANI_SLOT_DB[j]] = ANI_JUDGE_DB[cv]            

    if p2_judge_ani != {}:
        player_status['p2_judge_ani'] = p2_judge_ani

  
    p1_lane_ani = {}
    for j in range(0,8):
        cv = packet[0x9C4+j]  
        if cv != 0:
            p1_lane_ani[ANI_SLOT_DB[j]] = 1
          
    if p1_lane_ani != {}:
        player_status['p1_lane_ani'] = p1_lane_ani
    
    p2_lane_ani = {}
    for j in range(0,8):
        cv = packet[0x9CC+j]  
        if cv != 0:
            p2_lane_ani[ANI_SLOT_DB[j]] = 1
          
    if p2_lane_ani != {}:
        player_status['p2_lane_ani'] = p2_lane_ani    
 
    p1_note_ani = {}
    for j in range(0,8):
        cv = packet[0x9D4+j]  
        if cv != 0:
            p1_note_ani[ANI_SLOT_DB[j]] = 1
          
    if p1_note_ani != {}:
        player_status['p1_note_ani'] = p1_note_ani 
 
    p2_note_ani = {}
    for j in range(0,8):
        cv = packet[0x9DC+j]  
        if cv != 0:
            p2_note_ani[ANI_SLOT_DB[j]] = 1
          
    if p2_note_ani != {}:
        player_status['p2_note_ani'] = p2_note_ani  
        
        
    p1_score = struct.unpack("<I",packet[0x9E4:0x9E8])[0]
    if p1_score != 0:
        player_status['p1_score'] = p1_score

    p2_score = struct.unpack("<I",packet[0x9E8:0x9EC])[0]
    if p2_score != 0:
        player_status['p2_score'] = p2_score
 
    p1_score2 = struct.unpack("<I",packet[0x9EC:0x9F0])[0]
    if p1_score2 != 0:
        player_status['p1_score2'] = p1_score2

    p2_score2 = struct.unpack("<I",packet[0x9F0:0x9F4])[0]
    if p2_score2 != 0:
        player_status['p2_score2'] = p2_score2 
        
        
    idkpad2 = struct.unpack("<I",packet[0x9F4:0x9F8])[0]
    if idkpad2 != 0:
        player_status['idkpad2'] = idkpad2
    
    p1_life = struct.unpack("<H",packet[0x9F8:0x9FA])[0]
    if p1_life != 0:
        player_status['p1_life'] = p1_life

    p2_life = struct.unpack("<H",packet[0x9FA:0x9FC])[0]
    if p2_life != 0:
        player_status['p2_life'] = p2_life
 
    p1_life2 = struct.unpack("<H",packet[0x9FC:0x9FE])[0]
    if p1_life2 != 0:
        player_status['p1_life2'] = p1_life2

    p2_life2 = struct.unpack("<H",packet[0x9FE:0xA00])[0]
    if p2_life2 != 0:
        player_status['p2_life2'] = p2_life2     
 

    if(player_status != {}):
        events['Player'] = player_status
     
    
    
    return events
    
    
    
if __name__ == "__main__":
    packets = get_song_packets_from_dump(sys.argv[1])
    print(f"Read {len(packets)} Song Packets from Dump.")
    
    songs = carve_songs_from_song_packets(packets)
    print(f"Read {len(songs)} Songs from Dump.")
    
    for song_token in songs.keys():
        print("---- Parsing Song with Token: %s ----" % song_token)
        print(json.dumps(songs[song_token]['info'],sort_keys=True, indent=4))
        for i in range(0,len(songs[song_token]['packets'])):
            packet_ts = songs[song_token]['packets'][i][0]
            cpacket = songs[song_token]['packets'][i][1]
            packet_info = get_packet_event_info(cpacket)
            print("]====[Packet %d: %d]====[" % (i,packet_ts))
            for sec in packet_info.keys():
                
                if(sec == 'p1_holdcombo'):
                    if(packet_info[sec] == 0):
                        continue
                print("%s: " % sec)                        
                print(packet_info[sec])
            print("]====END PACKET====[")
            #exit(1)
              
        print("---- End Song ----")
    