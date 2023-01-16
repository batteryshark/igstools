# Reconstruct Game ELF 
import struct,os,sys


SKIP_OFFSET = 0xFA
SKIP_MULT = 5
SKIP_MODE = 1

DEC_ORDER = [3,5,1,2,4,6]


g = open("sub","wb")

with open("head","rb") as f:
	g.write(f.read())
	
for item in DEC_ORDER:
	f = open("rc%d.d" % item,"rb")
	f.seek(SKIP_OFFSET * 1024)
	g.write(f.read())
	if(SKIP_MODE == 1):
		SKIP_OFFSET+=SKIP_MULT
	elif(SKIP_MODE == 2):
		SKIP_OFFSET-=SKIP_MULT
		
print("DONE!")