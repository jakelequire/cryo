#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>

const char *__c_I64ToHex(int64_t value)
{
    char *buffer = (char *)malloc(sizeof(char) * 1024);
    sprintf(buffer, "%lx", value);
    printf("Hex: 0x%s\n", buffer);
    const char *hex = (const char *)buffer;
    return hex;
}
