
BASE_PATH := /peng/linux
STANDARD_FLAGS := -shared -m32 -fpack-struct=1 -fno-stack-protector
BASE_COMPONENTS := src/pm_patch.c src/patches/patches.c src/utils/utils.c src/log/a27_log.c src/io/*.c src/a27/utils.c
EMULATOR_COMPONENTS := src/a27/emulator.c src/a27/firmware/*.c src/a27/song/*.c

# This is the standard patch, it includes emulator components to emulate the A27 ASIC as well.
pm_patch:
	cc -DA27_EMU_SUPPORTED $(STANDARD_FLAGS) $(BASE_COMPONENTS) $(EMULATOR_COMPONENTS) -lpthread -o $(BASE_PATH)/pm_patch.so
	
# The "NoEmu" Variant is for a real PC and supports IO Emulation alongside logging and the various patches.
pm_patch_noemu:
	cc $(STANDARD_FLAGS) $(BASE_COMPONENTS) -lpthread -o $(BASE_PATH)/pm_patch_noemu.so
	
clean:
	rm $(BASE_PATH)/pm_patch*.so
