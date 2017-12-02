#!/bin/bash

mkdir build
cd build
cmake -Dtest=ON ..
make