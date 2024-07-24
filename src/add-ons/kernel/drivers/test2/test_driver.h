#ifndef _TEST_DRIVER_H
#define _TEST_DRIVER_H
#include <Drivers.h>
#include <Errors.h>
#include <KernelExport.h>
#include <kernel.h>
#include <stdlib.h>
#include <string.h>
/*** ioctl用パラメータ(第3引数)の定義 ***/
struct mydevice_values {
  int val1;
  int val2;
};

enum testdevice_state {
  TESTDEVICE_STATE_DISABLE=1,
  TESTDEVICE_STATE_ENABLE=2,
};



/* デバドラに値を設定するコマンド。パラメータはmydevice_values型ではない｡ */
#define MYDEVICE_SET_VALUES B_DEVICE_OP_CODES_END+ 1
/* デバドラから値を取得するコマンド。パラメータはmydevice_values型ではない｡ */
#define MYDEVICE_GET_VALUES B_DEVICE_OP_CODES_END+2

#define TESTDEVICE_STATE_WRITE B_DEVICE_OP_CODES_END+3

#define TESTDEVICE_STATE_READ B_DEVICE_OP_CODES_END+4

#define TESTDEVICE_FIFO_CLEAN B_DEVICE_OP_CODES_END+5
#endif // _TEST_DRIVER_H
