#
# SDRPortal Makefile
#
# Ben Kempke
#

CC = gcc
CPPC = g++
THREAD_LIB = -pthread
SOCKET_HANDLERS = ./bin/obj/socket_handler.o
CLIENT_INT = ./bin/obj/clientInterface.o
BASE64_LIB = ./bin/obj/base64.o
SHA1_LIB = ./bin/obj/sha1.o
UHD_INT = ./bin/obj/uhd_int.o
INCLUDE = -Ilib

all: uhdd

uhdd: $(SOCKET_HANDLERS) $(CLIENT_INT) $(BASE64_LIB) $(SHA1_LIB) $(UHD_INT)
	$(CPPC) -g $(THREAD_LIB) $(INCLUDE) src/uhd_daemon.cc $(LITHIUM_UTILS) $(SOCKET_HANDLERS) $(CLIENT_INT) $(UHD_INT) $(BASE64_LIB) $(SHA1_LIB) -luhd -o ./bin/uhdd

$(SOCKET_HANDLERS): $(CLIENT_INT) src/socketInterface.cc lib/socketInterface.h
	$(CPPC) $(THREAD_LIB) $(INCLUDE) $(CLIENT_INT)  -c src/socketInterface.cc -o $(SOCKET_HANDLERS)

$(BASE64_LIB): src/base64.cc lib/base64.h
	$(CPPC) $(INCLUDE) -c src/base64.cc -o $(BASE64_LIB)

$(SHA1_LIB): src/sha1.cc lib/sha1.h
	$(CPPC) $(INCLUDE) -c src/sha1.cc -o $(SHA1_LIB)

$(CLIENT_INT): src/clientInterface.cc lib/clientInterface.h
	$(CPPC) $(INCLUDE) -c src/clientInterface.cc -o $(CLIENT_INT)

$(UHD_INT): src/uhd_interface.cc lib/uhd_interface.h
	$(CPPC) $(INCLUDE) -c src/uhd_interface.cc -o $(UHD_INT)

clean:
	rm -f ./bin/obj/*.o

