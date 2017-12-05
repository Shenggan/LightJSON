#!/bin/bash

cd build
./UnitTests
cd ../example
mkdir build
cd build
cmake ..
make
./c_style
./class_style