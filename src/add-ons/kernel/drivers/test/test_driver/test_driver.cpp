/*
 * Copyright 2024, Yuki Kogi(nilpe), admini@nilpe.net.
 *
 * Distributed under the terms of the MIT License.
 *
 * test driver.
 */
#include <Drivers.h>
#include <Errors.h>
#include <KernelExport.h>
#include <PCI.h>
#include <bus/PCI.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "test_driver.h"

// #define TRACE_TEST_DRIVER
#ifdef TRACE_TEST_DRIVER
#define TRACE(x...) dprintf("test_driver: " x)
#else
#define TRACE(x...)
#endif
#define ERROR(x...) dprintf("test_driver: " x)
_EXPORT int32 api_version = B_CUR_DRIVER_API_VERSION;

status_t init_hardware(void) {
  TRACE("init_hardware\n");
  return B_OK;
}
status_t init_driver(void) {
  TRACE("init_driver\n");
  return B_OK;
}
void uninit_driver(void) {
  TRACE("uninit_driver\n");
  return;
}
const char **publish_devices(void) {
  static const char *devices[] = {"test_driver", NULL};
  return devices;
}
static status_t test_open(const char *name, uint32 flags,
                          driver_cookie **out_cookie) {
  TRACE("open\n");
}
static device_hooks test_driver_hook = {(device_open_hook)test_open,
                                        (device_close_hook)NULL,
                                        (device_free_hook)NULL,
                                        (device_control_hook)NULL,
                                        (device_read_hook)NULL,
                                        (device_write_hook)NULL,
                                        NULL,
                                        NULL,
                                        NULL,
                                        NULL};

device_hooks *find_device(const char *name) {
  // assert(name != NULL);

  return &test_driver_hook;
}
