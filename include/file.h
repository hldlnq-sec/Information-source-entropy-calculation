#ifndef FILE_H
#define FILE_H

#include <stddef.h>

int read_file_bytes(const char *path, unsigned char **buffer, size_t *size);

#endif