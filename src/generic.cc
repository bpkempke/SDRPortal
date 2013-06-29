#include <string>
#include <generic.h>

//TODO: These error checkers aren't doing their job...
bool isInteger(std::string in_string){
	return true;
}

bool isDouble(std::string in_string){
	return true;
}

primType stringToPrim(std::string in_string){
	if(in_string == "DOUBLE")
		return DOUBLE;
	else if(in_string == "FLOAT")
		return FLOAT;
	else if(in_string == "INT") 
		return INT;
	else if(in_string == "INT32")
		return INT32;
	else if(in_string == "INT16")
		return INT16;
	else if(in_string == "INT8")
		return INT8;
	else if(in_string == "UINT32")
		return UINT32;
	else return VOID;
}

/*void setParamIntegerNoClip(){

}

void setParamDoubleClip(std::string arg){
	if(isDouble(arg)){
		double param_req = strtod(arg);
		double param_clipped = uhd_int->clipRXFreq(param_req);
		if(param_req == param_clipped)
			uhd_int->setRXFreq(param_req);
		else
			throw badArgumentException(OUT_OF_BOUNDS, 1, arg1);
	} else 
		throw malformedArgumentException(1, arg1);
}*/
