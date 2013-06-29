#ifndef UHD_INTERFACE_H
#define UHD_INTERFACE_H

#include "genericSDRInterface.h"
#include "hierarchicalDataflowBlock.h"
#include "streamConverter.h"
#include <uhd/usrp/multi_usrp.hpp>
#include <uhd/utils/thread_priority.hpp>
#include <map>
#include <vector>

class uhdInterface : public genericSDRInterface{
public:
	//Constructor
	uhdInterface(std::string args, std::string tx_subdev, std::string rx_subdev, std::string tx_ant, std::string rx_ant, double tx_rate, double rx_rate, double tx_freq, double rx_freq, double tx_gain, double rx_gain, bool codec_highspeed);

	//Transmit data methods
	void txStart(int in_chan){};
	void txEnd(int in_chan);

	//Receive data methods
	void rxStart(int in_chan);
	int rxData(std::complex<int16_t> *rx_data_iq, int num_samples, int rx_chan);
	void rxEnd();
	void *rxThread(int rx_chan);
	void *txThread(int tx_chan);

	//Certain functions inherited from genericSDRInterface class
	virtual void setRXFreq(paramData in_param);
	virtual void setTXFreq(paramData in_param);
	virtual void setRXGain(paramData in_param);
	virtual void setTXGain(paramData in_param);
	virtual void setRXRate(paramData in_param);
	virtual void setTXRate(paramData in_param);
	virtual paramData getRXFreq(rxtxChanInfo in_chan);
	virtual paramData getTXFreq(rxtxChanInfo in_chan);
	virtual paramData getRXGain(rxtxChanInfo in_chan);
	virtual paramData getTXGain(rxtxChanInfo in_chan);
	virtual paramData getRXRate(rxtxChanInfo in_chan);
	virtual paramData getTXRate(rxtxChanInfo in_chan);
	virtual bool checkRXChannel(int in_chan);
	virtual bool checkTXChannel(int in_chan);
	virtual void openRXChannel(int in_chan);
	virtual void openTXChannel(int in_chan);
	virtual bool checkRXFreq(paramData in_param);
	virtual bool checkTXFreq(paramData in_param);
	virtual bool checkRXGain(paramData in_param);
	virtual bool checkTXGain(paramData in_param);
	virtual bool checkRXRate(paramData in_param);
	virtual bool checkTXRate(paramData in_param);
	virtual void setCustomSDRParameter(std::string name, std::string val, int in_chan);
	virtual void txIQData(void *data, int num_bytes, int tx_chan, primType in_type);


private:
	void writeMAX2829Reg(uint32_t value);
	//Pointer to the created uhd object.  TODO: Maybe this should be a superclass to make things more general?
	uhd::usrp::multi_usrp::sptr shared_uhd;

	//Pointers to the tx and rx streams
	std::vector<uhd::tx_streamer::sptr> tx_streams;
	std::vector<uhd::rx_streamer::sptr> rx_streams;
	std::vector<uhd::tx_metadata_t> tx_md;
	std::vector<uhd::rx_metadata_t> rx_md;

	//Queue of samples to transmit
	std::vector<std::vector<std::complex<float> > > tx_queue;

	//PThreads for rx and tx
	pthread_t rx_listener, tx_thread;

	//Stream converter to easily switch back and forth between primitive streaming types...
	streamConverter<int16_t> str_converter;

	//Special parameters just for the USRP series
	//TODO: This...

};

#endif

