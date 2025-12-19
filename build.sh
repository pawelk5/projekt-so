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
)