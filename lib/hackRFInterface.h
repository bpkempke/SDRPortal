#ifndef HACKRF_INTERFACE_H
#define HACKRF_INTERFACE_H

#include <queue>
#include "genericSDRInterface.h"
#include "genericSocketInterface.h"
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

	virtual void setTXFreq(paramData in_param);
	virtual void setTXGain(paramData in_param);
	virtual void setTXRate(paramData in_param);
	virtual paramData getTXFreq(rxtxChanInfo in_chan);
	virtual paramData getTXGain(rxtxChanInfo in_chan);
	virtual paramData getTXRate(rxtxChanInfo in_chan);
	virtual bool checkTXChannel(int in_chan);
	virtual void openTXChannel(int in_chan);
	virtual bool checkTXFreq(paramData in_param);
	virtual bool checkTXGain(paramData in_param);
	virtual bool checkTXRate(paramData in_param);
	virtual void txIQData(void *data, int num_bytes, int tx_chan, primType in_type);

	virtual void setCustomSDRParameter(std::string name, std::string val, int in_chan);

private:
	void handleHRFResult(const char *func_name, int result, bool force_kill);
	void stopTX();
	void startTX();

	hackrf_device *hrf_dev;
	bool is_receiving, is_transmitting;
	bool rx_cancel;
	pthread_t rx_listener;
	char *read_translate_buffer;
	int read_translate_length;

	//Variables to store current parameters
	uint32_t cur_rate, cur_rx_gain, cur_tx_gain;
	uint64_t cur_freq;

	//Stream converter to easily switch back and forth between primitive streaming types...
	streamConverter<char> str_converter;
	std::queue<char> tx_queue;
};

#endif
