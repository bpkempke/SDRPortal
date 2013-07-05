#ifndef GENERIC_H
#define GENERIC_H

#include <sstream>
#include <stdlib.h>

enum primType {DOUBLE, INT, FLOAT, INT32, INT16, INT8, UINT32, UINT64, C_STRING, VOID};

enum socketType{
	SOCKET_TCP,
	SOCKET_UDP,
	SOCKET_WS_TEXT,
	SOCKET_WS_BINARY
};

bool isInteger(std::string in_string);
bool isDouble(std::string in_string);
primType stringToPrim(std::string in_string);

class badArgumentException {
public:
	enum errorEnum { MALFORMED, OUT_OF_BOUNDS };
	badArgumentException(errorEnum error, int argnum, std::string argument_str){this->error = error; this->argnum = argnum; this->argument_str = argument_str;};
	const char* what() const{ 
		std::stringstream response;
		if(error == MALFORMED){
			response << "Malformed argument exception: Argument #" << argnum << ": " << argument_str;
		} else if(error == OUT_OF_BOUNDS){
			response << "Out of bounds argument exception: Argument #" << argnum << ": " << argument_str;
		}

		std::string ret = response.str();
		return ret.c_str();
	};
private:
	errorEnum error;
	int argnum;
	std::string argument_str;
};

class invalidCommandException {
public:
	invalidCommandException(std::string command_str){this->command_str = command_str;};
	const char* what() const{
		std::stringstream response;
		response << "Invalid command exception: " << command_str;

		std::string ret = response.str();
		return ret.c_str();
	};
private:
	std::string command_str;
};

#endif

