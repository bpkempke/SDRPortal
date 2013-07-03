#ifndef HACKRF_INTERFACE_H
#define HACKRF_INTERFACE_H

#include <vector>
#include "genericSDRInterface.h"
#include "hierarchicalDataflowBlock.h"
#include "streamConverter.h"
#include <hackrf.h>

class hackRFInterface : public genericSDRInterface{
public:
	hackRFInterface(int index);
	virtual ~hackRFInterface();

	int rxData(hackrf_transfer *in_transfer);

	//Certain functions inherited from genericSDRInterface class
	virtual void setRXFreq(paramData in_param);
	virtual void setRXGain(paramData in_param);
	virtual void setRXRate(paramData in_param);
	virtual paramData getRXFreq(rxtxChanInfo in_chan);
	virtual paramData getRXGain(rxtxChanInfo in_chan);
	virtual paramData getRXRate(rxtxChanInfo in_chan);
	virtual bool checkRXChannel(int in_chan);
	virtual void openRXChannel(int in_chan);
	virtual bool checkRXFreq(paramData in_param);
	virtual bool checkRXGain(paramData in_param);
	virtual bool checkRXRate(paramData in_param);
	virtual void setCustomSDRParameter(std::string name, std::string val, int in_chan);

private:
	hackrf_device *hrf_dev;
	bool is_receiving;
	bool rx_cancel;
	pthread_t rx_listener;

	//Stream converter to easily switch back and forth between primitive streaming types...
	streamConverter<char> str_converter;
};

#endif
