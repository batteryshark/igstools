# IGS Game Image V1 - Game Data Extractor [Incomplete]
# shfs Filesystem (GameData for Submarine Crisis)
import os,sys,struct,zlib,datetime

EXPECTED_MAGIC = ' <International Games System Co., Ltd.> Game Image '
PATH_TO_IMAGE = None

IMAGE_HEADER = {}
IMAGE_STATS_TABLE = []

def read_zlib_chunk(fp,offset=None,amt=None):
    if offset:
        fp.seek(offset)
    if not amt:
        amt = struct.unpack("<H",fp.read(2))[0]
    return zlib.decompress(fp.read(amt))
    


def dump_file(start_offset,chunk_list,out_name):
    file_data = b""
    with open(PATH_TO_IMAGE,"rb") as f:
        f.seek(start_offset)
        for entry in chunk_list:
            if(entry > 0x01000000):
                #print("Read @ %04X Data Amount: %04X" % (f.tell(),(entry - 0x01000000)))
                file_data += f.read(entry - 0x01000000)
            else:
                #print("Decompress @ %04X Data Amount: %04X" % (f.tell(),entry))
                file_data += read_zlib_chunk(f,None,entry)
    if(file_data.startswith(b"\x7F\x45\x4C\x46")):
        out_name += ".elf"
    if(file_data.startswith(b"\x52\x49\x46\x46")):
        out_name += ".wav"
    with open(out_name,"wb") as g:
        g.write(file_data)


def get_header():
    with open(PATH_TO_IMAGE,"rb") as f:
        IMAGE_HEADER["idk_1"] = struct.unpack("<I",f.read(4))[0]
        IMAGE_HEADER["magic"] = f.read(64).replace(b"\x00",b"").decode('ascii')
        IMAGE_HEADER["page_size"] = struct.unpack("<I",f.read(4))[0]
        IMAGE_HEADER["idk_2"] = struct.unpack("<I",f.read(4))[0]
        IMAGE_HEADER["idk_3"] = struct.unpack("<I",f.read(4))[0]
        IMAGE_HEADER["idk_4"] = struct.unpack("<I",f.read(4))[0]
        IMAGE_HEADER["idk_5"] = struct.unpack("<I",f.read(4))[0]
        IMAGE_HEADER["idk_6"] = struct.unpack("<I",f.read(4))[0]
        IMAGE_HEADER["idk_7"] = struct.unpack("<I",f.read(4))[0]
        IMAGE_HEADER["idk_8"] = struct.unpack("<I",f.read(4))[0]
        IMAGE_HEADER["idk_9"] = struct.unpack("<I",f.read(4))[0]
        IMAGE_HEADER["file_stat_table_offset"] = struct.unpack("<I",f.read(4))[0]
        IMAGE_HEADER["file_name_table_offset"] = struct.unpack("<I",f.read(4))[0]
        IMAGE_HEADER["idk_10"] = struct.unpack("<I",f.read(4))[0]      
        IMAGE_HEADER["idk_11"] = struct.unpack("<I",f.read(4))[0]        
        IMAGE_HEADER["idk_12"] = f.read(3)     
        IMAGE_HEADER["idk_13"] = struct.unpack("<I",f.read(4))[0]        
        IMAGE_HEADER["cluster_size"] = struct.unpack("<I",f.read(4))[0]
        IMAGE_HEADER["num_tail_entries"] = struct.unpack("<I",f.read(4))[0]
        IMAGE_HEADER["tail_table_offset"] = struct.unpack("<I",f.read(4))[0]

def parse_file_name_table():
    raw_name_table = b""
    with open(PATH_TO_IMAGE,"rb") as f:
        f.seek(IMAGE_HEADER['file_name_table_offset'])
        #TODO Figure out where the size of the name table is stored.
        while f.tell() < IMAGE_HEADER['tail_table_offset'] - (IMAGE_HEADER["num_tail_entries"] * 8):
            try:
                raw_name_table+=read_zlib_chunk(f,None,None)
            except:
                break
    
    print("Name Table Size: %d" % len(raw_name_table))
    # TODO: Actually Parse the Names
    
    
def parse_stats_table():
    raw_stats_table = b""
    with open(PATH_TO_IMAGE,"rb") as f:
        f.seek(IMAGE_HEADER['file_stat_table_offset'])
        #TODO Figure out where the size of the name table is stored.
        while f.tell() < IMAGE_HEADER['file_name_table_offset']:
            try:
                raw_stats_table+=read_zlib_chunk(f,None,None)
            except:
                break
    
    print("Stats Table Size: %d" % len(raw_stats_table))    
    soff = 0
    file_counter = 0
    entry_counter = 0
    while soff < len(raw_stats_table):
        entry_counter+=1
        last_start = soff
        entry_type = struct.unpack("<I",raw_stats_table[soff:soff+4])[0]
        soff+=4
        if entry_type == 0xFF001FF3:
            path_len = struct.unpack("<H",raw_stats_table[soff:soff+2])[0]
            soff+=2
            path_str = raw_stats_table[soff:soff+path_len]
            soff+=path_len
            IMAGE_STATS_TABLE.append({
            'entry_offset':last_start,
            'entry_count':entry_counter,
            'entry_type':"LINK",
            'path_str':path_str
            })
        elif entry_type == 0xFF001ED2 or entry_type == 0xFF0016D2:
            if(entry_type == 0xFF001ED2):
                entry_type_name = 'IDK_1ED2'
            elif(entry_type == 0xFF0016D2):
                entry_type_name = 'IDK_16D2'
                
            ne = {
            'entry_offset':last_start,
            'entry_count':entry_counter,
            'entry_type':entry_type_name
            }
            ne['timestamp'] = datetime.datetime.fromtimestamp(struct.unpack("<I",raw_stats_table[soff:soff+4])[0])
            soff+=4
            ne['start_offset'] = struct.unpack("<I",raw_stats_table[soff:soff+4])[0]
            soff+=4
            ne['idk_1'] = struct.unpack("<I",raw_stats_table[soff:soff+4])[0]
            soff+=4
            # Sometimes this entry looks like this - it's not a file, or it has no idea, idk what it is.
            # Actually yeah these might be files... they have offsets and some of them have a chunk value 
            # But it's not the right amount so idk
            # Value IDK3 has a size though...
            if(ne['idk_1'] != 0xFFFFFFFF):
                ne['entry_type'] += "_%d" % ne['idk_1']
                ne['idk_2'] = struct.unpack("<I",raw_stats_table[soff:soff+4])[0]
                soff+=4
                ne['idk_3'] = struct.unpack("<I",raw_stats_table[soff:soff+4])[0]
                soff+=4        
                
            elif ne['idk_1'] == 0xFFFFFFFF:
                file_counter+=1
                ne['file_count'] = file_counter
                ne['idk_2'] = struct.unpack("<I",raw_stats_table[soff:soff+4])[0]
                soff+=4
                ne['idk_h3'] = struct.unpack("<H",raw_stats_table[soff:soff+2])[0]
                soff+=2
                ne['num_blocks'] = struct.unpack("<H",raw_stats_table[soff:soff+2])[0] + 1
                soff+=2
                ne['data_blocks'] = []
                for i in range(0,ne['num_blocks']):
                    ne['data_blocks'].append(struct.unpack("<I",raw_stats_table[soff:soff+4])[0])
                    soff+=4
                print(ne)
                file_name = ne['entry_type'] + "_%d" % file_counter
                dump_file(ne['start_offset'],ne['data_blocks'],file_name)
            else:
                print("Offset: %04X Unknown SubType %04X" % (last_start,ne['idk_1']))
                exit(-1)
                
            IMAGE_STATS_TABLE.append(ne)
            print(ne)
                   
        elif entry_type == 0xFF001ED1:
            idk_1 = struct.unpack("<I",raw_stats_table[soff:soff+4])[0]
            soff+=4
            dt = datetime.datetime.fromtimestamp(struct.unpack("<I",raw_stats_table[soff:soff+4])[0])
            soff+=4
            idk_2 = raw_stats_table[soff:soff+3]
            soff+=3
            IMAGE_STATS_TABLE.append({
            'entry_offset':last_start,
            'entry_count':entry_counter,
            'entry_type':"IDK_1ED1",
            'idk_1':idk_1,
            'idk_2':idk_2,
            'timestamp':dt       
            })               
        
        else:
            print("Unhandled Entry Type: %04X @ %04X" % (entry_type,soff-4))
            exit(1)
              











def usage():
    print("Usage: %s in_file" % (sys.argv[0]))
    exit(-1)


if __name__=="__main__":
    if len(sys.argv) < 2:
        usage()
    if not os.path.exists(sys.argv[1]):
        usage()
    PATH_TO_IMAGE = sys.argv[1]
    
    get_header() 
    if(IMAGE_HEADER['magic'] != EXPECTED_MAGIC):
        print("Invalid Magic for this Type")
        exit(-1)
        
        
    parse_file_name_table()
    parse_stats_table()