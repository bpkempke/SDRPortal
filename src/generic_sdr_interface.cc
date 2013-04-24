
void genericSDRInterface::setSDRParameter(std::string name, std::string val){
	//TODO: There's still too much code here.  Really could be cleaned up with some function pointers...?

	//Go through and figure out if this is a generic integer argument
	if(name == "RXCHANNEL" || name == "TXCHANNEL"){
		int val_int;
		if(isInteger(val))
			val_int = strtol(arg.c_str(), NULL);
		else
			throw badArgumentException(MALFORMED, val);

		//Check to see if the parameter passed is valid
		if(name == "RXCHANNEL" && checkRXChannel(val_int)) setRXChannel(val_int);
		else if(name == "TXCHANNEL" && checkTXChannel(val_int)) setTXChannel(val_int);
		else 
			throw badArgumentException(OUT_OF_BOUNDS, val);

	//Next check for generic double argument
	} else if(name == "RXFREQ" || name == "TXFREQ" || 
	          name == "RXGAIN" || name == "TXGAIN" || 
		  name == "RXRATE" || name == "TXRATE"){
		double val_double;
		if(isDouble(val_double))
			val_double = strtod(arg.c_str(), NULL);
		else
			throw badArgumentException(MALFORMED, val);

		//Check to see if the parameter passed is valid
		if(name == "RXFREQ" && checkRXFreq(val_double)) setRXFreq(val_double);
		else if(name == "TXFREQ" && checkTXFreq(val_double)) setTXFreq(val_double);
		else if(name == "RXGAIN" && checkRXGain(val_double)) setRXGain(val_double);
		else if(name == "TXGAIN" && checkTXGain(val_double)) setTXGain(val_double);
		else if(name == "RXRATE" && checkRXRate(val_double)) setRXRate(val_double);
		else if(name == "TXRATE" && checkTXRate(val_double)) setTXRate(val_double);
		else
			throw badArgumentException(OUT_OF_BOUNDS, val);
	} else {
		//Must be a non-standard command?  Forward on to the custom parameter handler...
		setCustomSDRParameter(name, val);
	}
}
