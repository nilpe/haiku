```bash
sudo apt install git nasm bc autoconf automake texinfo flex bison gawk build-essential unzip wget zip less zlib1g-dev libzstd-dev xorriso libtool gcc-multilib python3

sudo apt-get install u-boot-tools util-linux

mkdir ~/Code
cd Code
git clone <https://review.haiku-os.org/buildtools>
cd buildtool/jam
make -j16
sudo ./jam0 install
cd ../..
git clone <https://github.com/nilpe/haiku.git> --depth=3
curl -Lo "haiku/.git/hooks/commit-msg" https://review.haiku-os.org/tools/hooks/commit-msg
chmod +x haiku/.git/hooks/commit-msg
cd haiku

mkdir generated.x86_64; cd generated.x86_64
../configure -j16 --cross-tools-source ../../buildtools --build-cross-tools x86_64

# 最初はエラーが出る
#jam -q -j16 @nightly-anybootだとバグる
jam -q -j16 @release-vmware

#二回目
jam -q -j16 @release-vmware
```

```
fatal: No names found, cannot describe anything.
Error: you are using a Haiku clone without tags, please set
       the revision tag to use (e.g. HAIKU_REVISION=hrev43210)

../build/scripts/determine_haiku_revision .. build/haiku-revision

...failed DetermineHaikuRevision1 build/haiku-revision ...

```

と言われた場合は､

```bash
git tag -a hrev65432 -m "Initial tag"
git push origin hrev65432
export HAIKU_REVISION=hrev65432
```

するとなんとかなる
バージョンおかしいよ!って言われた場合はターミナルを立ち上げ直すとなんとかなる
aaa
ビルドができることを確認したら､

headers/private/system/syscalls.h:655付近に､

```c
extern int64 _kern_hello(int64 value);
```

headers/private/kernel/test.hを作って､

```c
# ifndef TEST_H

# define TEST_H

# include <OS.h>

extern int64 _user_hello(int64 value);

# endif
```

src/system/kernel/syscalls.cpp:57付近に､

```c
# include <test.h>
```

を追記し､
src/system/kernel/test.cppを作成｡
内容は

```C
#include <test.h>

int64 _user_hello(int64 value)
{
    return value;
}
 

```

src/system/kernel/Jamfile:59付近にtest.cppを追加｡
generated.x86_64/objects/haiku/x86_64/common/system/libroot/os/syscalls.S.inc
に､SYSCALL2(_kern_hello, 287)が存在することを確認｡
headers/posix/unistd.hにプロトタイプを追加｡
ビルドして､いい感じのソースコードで検証｡

# デバイスドライバの作成
src/add-ons/kernel/drivers/common/test1.cを追加し､
```
/*
 * Copyright 2002-2006, Haiku, Inc. All Rights Reserved.
 * Distributed under the terms of the MIT License.
 *
 * Copyright 2001-2002, Travis Geiselbrecht. All rights reserved.
 * Distributed under the terms of the NewOS License.
 */


#include <Drivers.h>
#include <string.h>


#define DEVICE_NAME "test1"

int32 api_version = B_CUR_DRIVER_API_VERSION;


static status_t
null_open(const char *name, uint32 flags, void **cookie)
{
	*cookie = NULL;
	return B_OK;
}


static status_t
null_close(void *cookie)
{
	return B_OK;
}


static status_t
null_freecookie(void *cookie)
{
	return B_OK;
}


static status_t
null_ioctl(void *cookie, uint32 op, void *buffer, size_t length)
{
	return EPERM;
}


static status_t
null_read(void *cookie, off_t pos, void *buffer, size_t *_length)
{
	*_length = 0;
	return B_OK;
}


static status_t
null_write(void *cookie, off_t pos, const void *buffer, size_t *_length)
{
	return B_OK;
}


//	#pragma mark -


status_t
init_hardware()
{
	return B_OK;
}


const char **
publish_devices(void)
{
	static const char *devices[] = {
		DEVICE_NAME, 
		NULL
	};

	return devices;
}


device_hooks *
find_device(const char *name)
{
	static device_hooks hooks = {
		&null_open,
		&null_close,
		&null_freecookie,
		&null_ioctl,
		&null_read,
		&null_write,
		/* Leave select/deselect/readv/writev undefined. The kernel will
		 * use its own default implementation. The basic hooks above this
		 * line MUST be defined, however. */
		NULL,
		NULL,
		NULL,
		NULL
	};

	if (!strcmp(name, DEVICE_NAME))
		return &hooks;

	return NULL;
}


status_t
init_driver(void)
{
	return B_OK;
}


void
uninit_driver(void)
{
}


```


を書き込む｡

src/add-ons/kernel/drivers/common/Jamfileに､
```
KernelAddon test1 :
	test1.c
	;
```
を追記｡

build/jam/packages/haiku:87あたりに色々乗ってる
AddDriversToPackage					: console dprintf null test1 <driver>tty usb_modeswitch zero ;
と書き換える