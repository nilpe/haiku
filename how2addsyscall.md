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

# デバイスドライバの作成

src/add-ons/kernel/drivers/test/をMkdir
src/addons/kernel/drivers/Jamfileに以下を追記

```plain
SubInclude HAIKU_TOP src add-ons kernel drivers test ;
```

src/add-ons/kernel/drivers/test/Jamfileを作り､

```plaintext
SubDir HAIKU_TOP src add-ons kernel drivers test ;

SubInclude HAIKU_TOP src add-ons kernel drivers test test_driver ;


```

と書き込む｡
src/add-ons/kernel/drivers/test/test_driverをmkdir｡
src/add-ons/kernel/drivers/test/test_driver/Jamfileを作って､

```plaintext
SubDir HAIKU_TOP src add-ons kernel drivers test test_driver ;

UsePrivateKernelHeaders ;

KernelAddon pch_thermal :
 test_driver.cpp
 ;

```

を書き込む｡

build/jam/packages/haiku:87あたりに色々乗ってる
