#
# SDRPortal Makefile
#
# Ben Kempke
#

CC = gcc
CPPC = g++
THREAD_LIB = -pthread
SOCKET_HANDLERS = ./bin/obj/socket_handler.o
BASE64_LIB = ./bin/obj/base64.o
UHD_INT = ./bin/obj/uhd_int.o
GENERIC_LIB = ./bin/obj/generic.o
INCLUDE = -Ilib

all: uhdd

uhdd: $(SOCKET_HANDLERS) $(BASE64_LIB) $(GENERIC_LIB) $(UHD_INT) genericSDRInterface.cc genericSocketInterface.cc hierarchicalDataflowBlock.cc
	$(CPPC) -g $(THREAD_LIB) $(INCLUDE) src/uhd_daemon.cc $(SOCKET_HANDLERS) $(UHD_INT) $(BASE64_LIB) $(GENERIC_LIB) -luhd -o ./bin/uhdd

$(SOCKET_HANDLERS): src/socketInterface.cc lib/socketInterface.h
	$(CPPC) $(THREAD_LIB) $(INCLUDE) $(CLIENT_INT)  -c src/socketInterface.cc -o $(SOCKET_HANDLERS)

$(BASE64_LIB): src/base64.cc lib/base64.h
	$(CPPC) $(INCLUDE) -c src/base64.cc -o $(BASE64_LIB)

$(UHD_INT): src/uhdInterface.cc lib/uhdInterface.h
	$(CPPC) $(INCLUDE) -c src/uhdInterface.cc -o $(UHD_INT)

$(GENERIC_LIB): src/generic.cc lib/generic.h
	$(CPPC) $(INCLUDE) -c src/generic.cc -o $(GENERIC_LIB)

clean:
	rm -f ./bin/obj/*.o

