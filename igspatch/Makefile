
all: a27_emu a27_keyio a27_log igstools

a27_emu:
	cc -shared a27/a27_utils.c a27/a27_emu.c -o ~/peng/linux/a27_emu.so

a27_keyio:
	cc -shared a27/a27_utils.c a27/a27_keyio.c -o ~/peng/linux/a27_keyio.so

a27_log:
	cc -shared a27/a27_log.c -o ~/peng/linux/a27_log.so


igstools:
	cc -shared igstools.c -o ~/peng/linux/igstools.so

clean:
	rm ~/peng/linux/igstools.so
	rm ~/peng/linux/a27_log.so
	rm ~/peng/linux/a27_keyio.so
