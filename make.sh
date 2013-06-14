#!/bin/bash

g++ -Wall -g -pthread -Ilib src/*.cc -luhd -lrtlsdr -o ./bin/uhdd
