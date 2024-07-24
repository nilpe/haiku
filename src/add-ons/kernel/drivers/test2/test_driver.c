/*
../misc/test.cより雛形を拝借
以下はそれについていたcopyright
*/

/*
 * Copyright (c) 2007 Marcus Overhagen <marcus@overhagen.de>
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify,
 * merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */
#define FIFO_SIZE 1024
#include "test_driver.h"

static sem_id devMutex;
static sem_id rwMutex;
static char FIFO[FIFO_SIZE + 1];
static int charNum = 0;
static enum testdevice_state state = TESTDEVICE_STATE_ENABLE;
static struct mydevice_values stored_values;
int32 api_version = B_CUR_DRIVER_API_VERSION;
status_t copy_from_user(void *kernel, const void *user, int len);
status_t copy_to_user(void *user, const void *kernel, int len);

status_t copy_from_user(void *kernel, const void *user, int len) {
  if (user == NULL) {
    return B_BAD_VALUE;
  }

  if (!IS_USER_ADDRESS(user)) {
    return B_BAD_ADDRESS;
  }

  return user_memcpy(kernel, user, len);
}

status_t copy_to_user(void *user, const void *kernel, int len) {
  if (user == NULL) {
    return B_BAD_VALUE;
  }

  if (!IS_USER_ADDRESS(user)) {
    return B_BAD_ADDRESS;
  }

  return user_memcpy(user, kernel, len);
}

status_t init_hardware(void) {
  dprintf("test: init_hardware\n");
  return B_OK;
}

status_t init_driver(void) {
  dprintf("test: init_driver\n");
  devMutex = create_sem(1, "devMutex");
  if (devMutex < B_OK) {
    return B_NO_MEMORY;
  }
  rwMutex = create_sem(1, "rwMutex");
  if (rwMutex < B_OK) {
    return B_NO_MEMORY;
  }
  memset(FIFO, 0, sizeof(FIFO));
  return B_OK;
}

void uninit_driver(void) {
  dprintf("test: uninit_driver\n");
  delete_sem(devMutex);
  delete_sem(rwMutex);
}

static status_t driver_open(const char *name, uint32 flags, void **_cookie) {
  dprintf("test: open\n");

  status_t ok = acquire_sem(devMutex);
  if (ok != B_OK) {
    return ok;
  }
  dprintf("test: open, success\n");
  return B_OK;
}

static status_t driver_close(void *cookie) {
  dprintf("test: close enter\n");
  release_sem(devMutex);
  dprintf("test: close leave\n");
  return B_OK;
}

static status_t driver_free(void *cookie) {
  dprintf("test: free\n");

  return B_OK;
}

static status_t driver_read(void *cookie, off_t position, void *buf,
                            size_t *num_bytes) {
  dprintf("test: read\n");

  dprintf("Reading from device\n");
  dprintf("charNum = %d\n", charNum);
  if (state == TESTDEVICE_STATE_DISABLE) {
    dprintf("Device is disabled\n");
    return B_IO_ERROR;
  }

  if (*num_bytes == 0 || charNum == 0) {
    return 0;
  }
  int Num2Read = min(*num_bytes, (size_t)charNum);

  if (Num2Read == 0) {
    return B_OK;
  } else if (Num2Read > FIFO_SIZE) {
    Num2Read = FIFO_SIZE;
  }
  char *kbuf = malloc(Num2Read);
  if (kbuf == NULL) {
    dprintf("Error in malloc\n");
    return B_NO_MEMORY;
  }
  for (int i = 0; i < Num2Read; i++) {
    kbuf[i] = FIFO[i];
  }
  // kbuf[Num2Read] = '\0';
  int t = copy_to_user(buf, kbuf, Num2Read);
  if (t != 0) {
    dprintf("Error in copy_to_user\n");
    Num2Read = Num2Read - t;
  }
  for (int i = 0; i < charNum - Num2Read; i++) {
    if (i + Num2Read >= FIFO_SIZE) {
      break;
    }
    FIFO[i] = FIFO[i + Num2Read];
  }
  charNum = charNum - Num2Read;
  *num_bytes = Num2Read;
  return B_OK;
}

static status_t driver_write(void *cookie, off_t position, const void *buffer,
                             size_t *num_bytes) {
  int length = *num_bytes;
  if (length > FIFO_SIZE - charNum) {
    length = FIFO_SIZE - charNum;
  }
  if (state == TESTDEVICE_STATE_DISABLE) {
    dprintf("Device is disabled\n");
    return B_IO_ERROR;
  }
  char *kbuf = malloc(length + 1);
  if (kbuf == NULL) {
    dprintf("Error in kmalloc\n");
    return B_NO_MEMORY;
  }
  int t = copy_from_user(kbuf, buffer, length);
  if (t != 0) {
    dprintf("Error in copy_from_user\n");
    return B_BAD_ADDRESS;
  }
  length = length - t;
  kbuf[length] = '\0';
  dprintf("Writing to device: %s\n", kbuf);
  acquire_sem(rwMutex);
  if (charNum + length + 1 > FIFO_SIZE) {
    dprintf("FIFO is full\n");
    free(kbuf);
    return B_BUFFER_NOT_AVAILABLE;
  }
  for (int i = 0; i < length; i++) {
    FIFO[charNum + i] = kbuf[i];
  }
  charNum = charNum + length;
  FIFO[charNum] = '\0';
  dprintf("FIFO: %s\n", FIFO);
  release_sem(rwMutex);

  *num_bytes = length;
  free(kbuf);
  return B_OK;
}

static status_t driver_control(void *cookie, uint32 op, void *arg, size_t len) {
   dprintf("test: control\n");

    switch (op) {
        case MYDEVICE_SET_VALUES:
            dprintf("MYDEVICE_SET_VALUES\n");
            return copy_from_user(&stored_values, arg, sizeof(stored_values)) ;
            
            break;

        case MYDEVICE_GET_VALUES:
            dprintf("MYDEVICE_GET_VALUES\n");
            return copy_to_user(arg, &stored_values, sizeof(stored_values));
            
            break;

        case TESTDEVICE_STATE_WRITE:
            dprintf("TESTDEVICE_STATE_WRITE\n");
            return copy_from_user(&state, arg, sizeof(state)) ;
           
            break;

        case TESTDEVICE_STATE_READ:
            dprintf("TESTDEVICE_STATE_READ\n");
            return copy_to_user(arg, &state, sizeof(state));
            
            break;

        case TESTDEVICE_FIFO_CLEAN:
            dprintf("TESTDEVICE_FIFO_CLEAN\n");
            acquire_sem(rwMutex);
            charNum = 0;
            FIFO[0] = '\0';
            release_sem(rwMutex);
            break;

        default:
            dprintf("unsupported command %u\n", op);
            return B_BAD_VALUE;
    }
    return B_OK;
}

const char **publish_devices(void) {
  static const char *names[] = {"test2/test/1", NULL};
  dprintf("test: publish_devices\n");
  return names;
}

device_hooks *find_device(const char *name) {
  static device_hooks hooks = {
      driver_open,    driver_close, driver_free,
      driver_control, driver_read,  driver_write,
  };
  dprintf("test: find_device\n");
  return &hooks;
}
