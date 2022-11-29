
BASE_PATH := /peng/linux

pm_patch:
	cc -shared src/igstools.c src/utils.c src/keyio.c src/patches.c src/a27/log.c src/a27/emu.c src/a27/utils.c src/a27/song.c -lpthread -o $(BASE_PATH)/igstools.so
	

clean:
	rm $(BASE_PATH)/igstools.so
