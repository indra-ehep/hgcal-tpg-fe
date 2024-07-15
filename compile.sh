#!/usr/bin/env bash
g++ -I common/inc emul_test-beam_Sep23.cpp  -l yaml-cpp `root-config --libs --cflags` -I`root-config --incdir` -o emul_test-beam_Sep23.exe
g++ -I common/inc findEMax.cpp  -l yaml-cpp `root-config --libs --cflags` -I`root-config --incdir` -o findEMax.exe
g++ -I common/inc read_econt_Jul24.cpp  -l yaml-cpp `root-config --libs --cflags` -I`root-config --incdir` -o read_econt_Jul24.exe
