#pragma once

unsigned int get_fuzz_input(void);
unsigned int get_emulated_drumio_state(unsigned int respect_debounce);
unsigned char get_emulated_coin_state(unsigned int respect_debounce);
unsigned short get_emulated_coin_counter(void);
void get_io_test_buffer(unsigned char* io_test_buffer);

void keyboardio_init(void);