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
cd haiku

mkdir generated.x86_64; cd generated.x86_64
../configure -j16 --cross-tools-source ../../buildtools --build-cross-tools x86_64

# 最初はエラーが出る
#jam -q -j16 @nightly-anybootだとバグる
jam -q -j16 @release-vmware
#二回目
jam -q -j16 @nightly-anyboot

```
