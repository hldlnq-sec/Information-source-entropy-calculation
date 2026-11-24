#include "../include/file.h"

#include <stdio.h>
#include <stdlib.h>

int read_file_bytes(const char *path, unsigned char **buffer, size_t *size)
{
    if (path == NULL || buffer == NULL || size == NULL) return -1;

    *buffer = NULL;
    *size = 0;

    FILE *file_p = fopen(path, "rb");
    if (file_p == NULL)
    {
        perror("fopen");
        return -1;
    }

    if (fseek(file_p, 0, SEEK_END) != 0)
    {
        perror("fseek");
        fclose(file_p);
        return -1;
    }

    long file_size = ftell(file_p);

    if (file_size < 0)
    {
        perror("ftell");
        fclose(file_p);
        return -1;
    }

    if (fseek(file_p, 0, SEEK_SET) != 0)
    {
        perror("fseek");
        fclose(file_p);
        return -1;
    }

    unsigned char *array = malloc(file_size > 0 ? (size_t)file_size : 1);

    if (array == NULL)
    {
        perror("malloc");
        fclose(file_p);
        return -1;
    }

    size_t read_count = fread(array, 1, (size_t)file_size, file_p);

    if (read_count != (size_t)file_size)
    {
        if (ferror(file_p))
        {
            perror("fread");
        }
        
        free(array);
        fclose(file_p);
        return -1;
    }

    if (fclose(file_p) != 0)
    {
        perror("fclose");
        free(array);
        return -1;
    }

    *buffer = array;
    *size = (size_t)file_size;

    return 0;
}