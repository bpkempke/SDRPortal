#!/bin/bash

NON_MAIN_FILES=`ls src/*.cc | grep -v _`
echo $NON_MAIN_FILES
g++ -std=c++0x -Wall -g -pthread -Ilib $NON_MAIN_FILES src/uhd_daemon.cc -luhd -lrtlsdr -lhackrf -o ./bin/uhdd
g++ -std=c++0x -Wall -g -pthread -Ilib $NON_MAIN_FILES src/shell_daemon.cc -luhd -lrtlsdr -lhackrf -o ./bin/shelld
