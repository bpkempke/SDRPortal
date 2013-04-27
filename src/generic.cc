#include <string>
#include <generic.h>

bool isInteger(std::string in_string){
	return true;
}

bool isDouble(std::string in_string){
	return true;
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
