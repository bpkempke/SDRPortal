#!/bin/bash

NON_MAIN_FILES=`ls src/*.cc | grep -v _`
echo $NON_MAIN_FILES
g++ -Wall -g -pthread -Ilib $NON_MAIN_FILES src/uhd_daemon.cc -luhd -lrtlsdr -o ./bin/uhdd
g++ -Wall -g -pthread -Ilib $NON_MAIN_FILES src/shell_daemon.cc -luhd -lrtlsdr -o ./bin/uhdd
