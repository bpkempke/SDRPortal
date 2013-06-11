#!/bin/bash

g++ -Wall -g -pthread -Ilib src/*.cc -luhd -o ./bin/uhdd
