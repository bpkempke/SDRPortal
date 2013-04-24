
enum {
	COMPLEX_FLOAT,
	COMPLEX_DOUBLE,
	COMPLEX_INT8,
	COMPLEX_INT16,
	COMPLEX_INT32
} stream_type;

class badArgumentException : public std::exception{
public:
	enum ERRORS{
		MALFORMED,
		OUT_OF_BOUNDS
	};
	badArgumentException(ERRORS error, int argnum, std::string argument_str){this->error = error; this->argnum = argnum; this->argument_str = argument_str}
	virtual const char* what() throw(){
		if(error == MALFORMED){
			response << "Malformed argument exception: Argument #" << argnum << ": " << argument_str;
		} else if(error == OUT_OF_BOUNDS){
			response << "Out of bounds argument exception: Argument #" << argnum << ": " << argument_str;
		}
		return response.str().c_str();
	}
private:
	ERRORS error;
	int argnum;
	std::string argument_str;
	std::stringstream response;
}

class invalidCommandException : public std::exception{
public:
	invalidCommandException(std::string command_str){this->command_str = command_str;}
	virtual const char* what() throw(){
		response << "Invalid command exception: " << command_str;
		return response.str().c_str();
	}
private:
	std::string command_str;
	std::stringstream response;
}
