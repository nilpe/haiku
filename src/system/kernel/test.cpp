#include <Errors.h>
#include <stdlib.h>
#include <test.h>

// 参考:src/system/kernel/disk_device_manager/ddm_userland_interface.cpp
template <typename T>
inline int copy_from_user(T *kernel, const T *user, int len) {
  if (user == NULL)
    return B_BAD_VALUE;

  if (!IS_USER_ADDRESS(user))
    return B_BAD_ADDRESS;

  return user_memcpy(kernel, user, len);
}

template <typename T>
inline int copy_to_user(T *user, const T *kernel, int len) {
  if (user == NULL) {
    return B_BAD_VALUE;
  }

  if (!IS_USER_ADDRESS(user)) {
    return B_BAD_ADDRESS;
  }

  return user_memcpy(user, kernel, sizeof(Type));
}

int _user_hello(int value) { return value; }
status_t _user_hashChar(const char *filename, unsigned long int len,
                        unsigned long int *value) {
  unsigned int rec_max = 255 * sizeof(char);
  char *fn = (char *)malloc(rec_max);
  if (fn == NULL) {
    // printk("kcalloc failed");
    return B_NO_MEMORY;
  }
  int willCopyByte = (rec_max < len) ? rec_max : len * sizeof(char);
  unsigned long ok = copy_from_user(fn, filename, willCopyByte);
  if (ok != 0) {
    // printk("copy_from_user failed");
    // printk("copy_from_user failed, %ld\n", ok);
    free(fn);
    return B_BAD_ADDRESS;
  }

  unsigned long int hash = 0xcbf29ce484222325;
  for (int j = 0; j < willCopyByte; j++) {
    hash = hash ^ fn[j];
    hash = hash * 0x100000001b3;
  }
  ok = copy_to_user(value, &hash, sizeof(long int));
  if (ok != 0) {
    // printk("copy_to_user failed");
    free(fn);
    return B_BAD_ADDRESS;
  }

  return 0;
}