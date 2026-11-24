#include <stdio.h>
#include <stdlib.h>
#include "include/file.h"

int main(void)
{
    unsigned char *data = NULL;
    size_t size = 0;

    if (read_file_bytes("data/test.txt", &data, &size) != 0)
    {
        printf("Ошибка чтения файла\n");
        return 1;
    }

    printf("Размер файла: %zu байт\n", size);

    for (size_t i = 0; i < size; i++)
    {
        printf("%02X ", data[i]);
    }

    printf("\n");

    free(data);

    return 0;
}
