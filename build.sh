#!/bin/bash

(
mkdir -p build
cd build
cmake -B . -S ..
cmake --build . --parallel $(nproc)

cd ..
mkdir -p bin
cp build/app/manager/park-manager bin/
cp build/app/main/park-main bin/
cp build/app/cashier/park-cashier bin/
cp build/app/restaurant/park-restaurant bin/
cp build/app/attraction/park-attraction bin/
cp build/app/client/park-client bin/
)