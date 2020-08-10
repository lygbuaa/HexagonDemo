#!/bin/bash
# set -ex
NDK=/home/hugoliu/Qualcomm/Hexagon_SDK/3.5.2/tools/android-ndk-r19c
toolchain_dir=/home/hugoliu/Qualcomm/Hexagon_SDK/3.5.2/tools/android-ndk-r19c/install/android-21/arch-arm64

# Create an arm64 API 26 libc++ toolchain.
# $NDK/build/tools/make-standalone-toolchain.sh \
# 	--arch=arm64 \
# 	--platform=android-21 \
# 	--toolchain=arm-linux-android-clang5.0 \
# 	--install-dir=$toolchain_dir/arm-21-toolchain-clang-64 \
# 	--use-llvm \
# 	--stl=gnustl 

$NDK/build/tools/make-standalone-toolchain.sh \
	--arch=arm64 \
	--platform=android-21 \
	--install-dir=$toolchain_dir/arm-21-toolchain-clang-64 \
	--toolchain=arm-linux-android-clang5.0
