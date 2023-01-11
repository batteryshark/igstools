#ifndef __KEY_EVDEV_H
#define __KEY_EVDEV_H

#include "keyio.h"
int evdev_try_open(void);
void evdev_input_loop(int kbd_evdev, PKeyIOState key_state, PKeyIOState last_key_state);
#endif
