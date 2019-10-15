#!/bin/bash
g++ -std=c++17 -O2 "source.cpp" -o "a.exe" -lpng || exit
"./$PROJ/a.exe"