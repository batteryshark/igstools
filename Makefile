
BASE_PATH := /peng/linux

igstools:
	cc -DA27_EMU_SUPPORTED -shared -m32 src/igstools.c src/utils.c src/keyio.c src/a27/song_manager.c src/a27/song_timer.c src/a27/song_utils.c src/patches.c src/a27/log.c src/a27/emu.c src/a27/utils.c src/a27/song.c src/a27/songtable.c -lpthread -o $(BASE_PATH)/igstools.so
	

igstools_noemu:
	cc -shared -m32 src/igstools.c src/utils.c src/keyio.c src/patches.c src/a27/log.c src/a27/utils.c -lpthread -o $(BASE_PATH)/igstools.so
	
clean:
	rm $(BASE_PATH)/igstools.so
