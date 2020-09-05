#ifndef _STDIO_H_GUARD
#define _STDIO_H_GUARD

#include <sys/cdefs.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

int printf(const char * __restrict, ...);
int putchar(int);
int puts(const char *);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _STDIO_H_GUARD */