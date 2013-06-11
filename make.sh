#!/bin/bash

g++ -g -pthread -Ilib src/*.cc -luhd -o ./bin/uhdd
