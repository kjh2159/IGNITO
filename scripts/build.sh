#!/bin/bash
mkdir ../build
cd ../build || exit

cmake ..\
	-DIGNITO_USE_SYSTEM=ON \
	-DPERFETTO=OFF \
	-DMV=OFF \
	-DCMAKE_BUILD_TYPE=Release \
	-DCMAKE_CXX_FLAGS="-D__ANDROID_UNAVAILABLE_SYMBOLS_ARE_WEAK__"

make -j$(nproc)
