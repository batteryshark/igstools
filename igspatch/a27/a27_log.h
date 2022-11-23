#pragma once

void a27log_init(void);
void a27log_lseek(int offset, int result);
void a27log_write(int result, unsigned int count, const void* buffer);
void a27log_read(int result, void* buffer);