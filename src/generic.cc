bool isInteger(string in_string){

}

bool isDouble(string in_string){

}

void setParamIntegerNoClip(){

}

void setParamDoubleClip(string arg){
	if(isDouble(arg)){
		double param_req = toDouble(arg);
		double param_clipped = uhd_int->clipRXFreq(param_req);
		if(param_req == param_clipped)
			uhd_int->setRXFreq(param_req);
		else
			throw badArgumentException(OUT_OF_BOUNDS, 1, arg1);
	} else 
		throw malformedArgumentException(1, arg1);
}
