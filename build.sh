#!/bin/bash

code="$PWD"
opts=-g
cd build > /dev/null
g++ $opts $code/test.cpp -o test.exe
cd $code > /dev/null
