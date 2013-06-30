#include <string>
#include <generic.h>
#include <stdlib.h>

//Basic logic behind isInteger and isDouble courtesy of:
//  http://pubs.opengroup.org/onlinepubs/009695399/functions/strtod.html
bool isInteger(std::string in_string){
	const char *s = in_string.c_str();
	char *endp;

	strtol(s, &endp, 0);
	if(s != endp && *endp == '\0')
		return true;
	else return false;
}

bool isDouble(std::string in_string){
	const char *s = in_string.c_str();
	char *endp;

	strtod(s, &endp);
	if(s != endp && *endp == '\0')
		return true;
	else return false;
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
