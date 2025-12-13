#!/bin/bash

(
mkdir -p build
cd build
cmake -B . -S ..
cmake --build . --parallel $(nproc)
)