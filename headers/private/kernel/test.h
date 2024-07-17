#ifndef TEST_H
#define TEST_H
#include <OS.h>

extern int _user_hello(int value);
extern status_t _user_hashChar(const char *filename, unsigned long int len,
                               unsigned long int *value);

#endif