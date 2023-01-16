import os,struct,sys,string,zlib

HEADER_NUM_ENTRIES_OFFSET = 0x64
HEADER_SIZE = 0xA8
INODE_SIZE = 16

def read_inode(f,is_root=False):
    ne = {'entry_offset':f.tell()}    
    ne['magic'] = struct.unpack("<I",f.read(4))[0]
    ne['offset_raw'] = struct.unpack("<I",f.read(4))[0]
    ne['stat_size'] = struct.unpack("<I",f.read(4))[0]
    # Calculate Required block Size
    pad = 4096 - (ne['stat_size'] % 4096) 
    if pad == 4096:
        pad = 0
    ne['padded_stat_size'] = pad + ne['stat_size']
    ne['num_blocks'] = int(ne['padded_stat_size'] / 4096)
 
    
    ne['idk'] = struct.unpack("<I",f.read(4))[0]
    ne['entry_size'] = INODE_SIZE
    if(ne['idk'] & 0x8000 > 0):
        ne['type'] = "FILE"
        ne['file_flag'] = (ne['offset_raw'] & 0xFF000000) >> 24
        ne['offset'] = (ne['offset_raw']) >> 4
    elif(ne['idk'] & 0x4000 > 0):
        ne['type'] = "DIR"
        ne['offset'] = (ne['offset_raw'] & 0x00FFFFFF) >> 4       
    else:
        ne['type'] = "FILE"
        ne['file_flag'] = (ne['offset_raw'] & 0xFF000000) >> 24
        ne['offset'] = (ne['offset_raw']) >> 4        
    
    if(is_root):        
        return ne
    # If we aren't reading a root, we have to also get the filename.
    # Read the first chunk
    ne['name'] = f.read(4)
    ne['entry_size']+=4
    if not ne['name'].endswith(b"\x00"):
        while 1:
            name_data = f.read(1)
            try:
                # Attempt to decode the symbol as ascii.
                nea = name_data.decode('ascii')
                ne['name'] += name_data
                ne['entry_size']+=1
            except:
                break

        # Back up 
        f.seek(-1,1)
        
        # Round to 4 byte boundary.
        location = f.tell()
        padding = 4 - (location % 4)
        if(padding > 0 and padding != 4):
            f.seek(padding,1)
            
    ne['name'] = ne['name'].replace(b"\x00",b"").decode('ascii')


    return ne
    
def read_inode_table(f):
    inode_table = []
    # Read number of entries
    f.seek(HEADER_NUM_ENTRIES_OFFSET)
    num_entries = struct.unpack("<I",f.read(4))[0]
    # Go to the end of the header
    f.seek(HEADER_SIZE)
    # Read the Root Inode
    inode_table.append(read_inode(f,True))
    # Read the rest of the Inodes
    for i in range(0,num_entries-1):
        inode_table.append(read_inode(f))
    return inode_table

def dump_file(f,offset,num_blocks,file_flag,output_path):
    # If it's an empty file, we'll just make it.
    file_data = b""
    if(offset != 0):
        print(f"Dumping File: {output_path}...")
        print("Offset: %04X" % offset)
        print("Num Blocks: %d" % num_blocks)
        print("File Flag: %04X" % file_flag)    
        f.seek(offset)
        block_offsets = []    
        for i in range(0,num_blocks):
            block_offsets.append(struct.unpack("<I",f.read(4))[0])

        for end_addr in block_offsets:
            block_size = end_addr - f.tell()
            file_data += zlib.decompress(f.read(block_size))
        
    with open(output_path,"wb") as g:
        g.write(file_data)

def parse_directory(f,inode_table,parent_index=0,base_path=""):
    parent_inode = inode_table[parent_index]

    print(f"Parsing Path: {base_path} from parent index: {parent_index}")
    print(parent_inode)
    
    # If this isn't a directory, we fucked up and need to error.
    if parent_inode['type'] != "DIR":
        print("Error: Parent Inode Is not A Directory!")
        exit(-1)
    
    # If this directory is empty, its offset will be 0. Just Return.
    if(parent_inode['offset'] == 0 or parent_inode['stat_size'] == 0):
        return
    
    # We need to find the first child entry index of this.
    first_child_entry_index = 0
    for i in range(0,len(inode_table)):
        if(parent_inode['offset'] == inode_table[i]['entry_offset']):
            first_child_entry_index = i
    if first_child_entry_index == 0:
        print("Error - Could not Find Child Entry Start Offset")
        exit(-1)

    # Find Last Child Index and Populate Child List    
    dest_offset = parent_inode['offset'] + parent_inode['stat_size']
    child_entries = []    
    for i in range(first_child_entry_index,len(inode_table)):
        if(inode_table[i]['entry_offset'] == dest_offset):
            break
        child_entries.append(inode_table[i])
    
    
    print("Path Contains: %d Children" % len(child_entries))        
    # Now We Iterate Every Entry
    for i in range(0,len(child_entries)):
        ce = child_entries[i]
        entry_path = os.path.join(base_path,ce['name'])
        print(entry_path)
        if ce['type'] == "DIR":
            print(entry_path)        
            if not os.path.exists(entry_path):
                os.makedirs(entry_path)
            
            # Recurse into this function
            parse_directory(f,inode_table,first_child_entry_index + i,entry_path)
        elif ce['type'] == "FILE":
            print(ce)
            dump_file(f,ce['offset'],ce['num_blocks'],ce['file_flag'],entry_path)

        


if __name__=="__main__":
    f = open("subhdc2","rb")
    inode_table = read_inode_table(f)
    print(f"{len(inode_table)} Inode Entries Read")
    parse_directory(f,inode_table,base_path=os.path.join(".","romfs"))