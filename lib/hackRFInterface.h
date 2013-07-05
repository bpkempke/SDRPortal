#ifndef HACKRF_INTERFACE_H
#define HACKRF_INTERFACE_H

#include <vector>
#include "genericSDRInterface.h"
#include "hierarchicalDataflowBlock.h"
#include "streamConverter.h"
#include <libhackrf/hackrf.h>

class hackRFInterface : public genericSDRInterface{
public:
	hackRFInterface();
	virtual ~hackRFInterface();

	int rxData(hackrf_transfer *in_transfer);
	int txData(hackrf_transfer *in_transfer);

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
	bool is_receiving, is_transmitting;
	bool rx_cancel;
	pthread_t rx_listener;
	char *read_translate_buffer;
	int read_translate_length;

	//Variables to store current parameters
	uint32_t cur_rate, cur_gain;
	uint64_t cur_freq;

	//Stream converter to easily switch back and forth between primitive streaming types...
	streamConverter<char> str_converter;
};

#endif
