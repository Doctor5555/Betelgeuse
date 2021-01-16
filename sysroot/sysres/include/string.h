#ifndef _STRING_H_GUARD
#define _STRING_H_GUARD

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*void strcpy(char *dest, char *source);
void strncpy(char *dest, char *source, size_t n);

void strcat(char* dest, char *source);
void strncat(char *dest, char *source, size_t n);

void strxfrm(char *string);*/

size_t strlen(const char *);

int memcmp(const void *, const void *, size_t);
void *memset(void *, const int , size_t);
void *memcpy(void * __restrict, const void * __restrict, size_t);
void *memmove(void *, const void *, size_t);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* _STRING_H_GUARD */