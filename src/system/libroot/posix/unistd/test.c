#include <unistd.h>

#include <errno.h>
#include <pthread.h>

#include <syscall_utils.h>

#include <errno_private.h>
#include <syscalls.h>

int hello(int value) { return _kern_hello(value); }

int hashChar(const char *filename, unsigned long int len,
             unsigned long int *value) {

  return _kern_hashChar(filename, len, value);
}