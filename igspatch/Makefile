BASE_PATH := /peng/linux
all: a27_emu a27_keyio a27_xkeyio a27_log igstools

a27_emu:
	cc -shared a27/a27_utils.c a27/a27_emu.c -o $(BASE_PATH)/a27_emu.so

a27_keyio:
	cc -shared a27/a27_utils.c a27/a27_keyio.c -o $(BASE_PATH)/a27_keyio.so

a27_xkeyio:
	cc -shared a27/a27_utils.c a27/a27_xkeyio.c -o $(BASE_PATH)/a27_xkeyio.so

a27_log:
	cc -shared a27/a27_log.c -o $(BASE_PATH)/a27_log.so


igstools:
	cc -shared igstools.c -o $(BASE_PATH)/igstools.so

clean:
	rm $(BASE_PATH)/igstools.so
	rm $(BASE_PATH)/a27_log.so
	rm $(BASE_PATH)/a27_keyio.so
	rm $(BASE_PATH)/a27_xkeyio.so	
	rm $(BASE_PATH)/a27_emu.so
