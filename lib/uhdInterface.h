#include "socketInterface.h"
#include "genericSDRInterface.h"
#include <uhd/usrp/multi_usrp.hpp>
#include <map>

//This is utilizing the 'Curiously Recurring Template Pattern'
class uhdInterface : public hierarchicalDataflowBlock, public genericSDRInterface{
public:
	//Constructor
	uhdInterface(std::string args, std::string tx_subdev, std::string rx_subdev, std::string tx_ant, std::string rx_ant, double tx_rate, double rx_rate, double tx_freq, double rx_freq, double tx_gain, double rx_gain, bool codec_highspeed);

	//Transmit data methods
	void txStart(int in_chan){};
	void txEnd(int in_chan);

	//Receive data methods
	void rxStart();
	int rxData(std::complex<int16_t> *rx_data_iq, int num_samples, int rx_chan);
	void rxEnd();
	void rxThread(int rx_chan);
	void txThread(int tx_chan);

protected:
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
	uhd::tx_streamer::sptr tx_stream;
	uhd::rx_streamer::sptr rx_stream;
	vector<uhd::tx_metadata_t> tx_md;
	vector<uhd::rx_metadata_t> rx_md;

	//Queue of samples to transmit
	std::vector<std::vector<std::complex<float> > > tx_queue;

	//PThreads for rx and tx
	pthread_t rx_listener, tx_thread;

	//maps used to identify the pointers coming up from downstream
	std::map<fdInterface*,uhdControlConnection*> control_connections;

	//Current RX/TX Channel configurations
	streamType cur_stream_type;

	//Special parameters just for the USRP series

//Virtual memembers inherited from genericSDRInterface
protected:
	void setRXFreq(paramData in_param){shared_uhd->set_rx_freq(in_param.getDouble(), in_param.getChannel());};
	void setTXFreq(paramData in_param){shared_uhd->set_tx_freq(in_param.getDouble(), in_param.getChannel());};
	void setRXGain(paramData in_param){shared_uhd->set_rx_gain(in_param.getDouble(), in_param.getChannel());};
	void setTXGain(paramData in_param){shared_uhd->set_tx_gain(in_param.getDouble(), in_param.getChannel());};
	void setRXRate(paramData in_param){shared_uhd->set_rx_rate(in_param.getDouble(), in_param.getChannel());};
	void setTXRate(paramData in_param){shared_uhd->set_tx_rate(in_param.getDouble(), in_param.getChannel());};
	paramData getRXFreq(int in_chan){return shared_uhd->get_rx_freq(in_chan);};
	paramData getTXFreq(int in_chan){return shared_uhd->get_tx_freq(in_chan);};
	paramData getRXGain(int in_chan){return shared_uhd->get_rx_gain(in_chan);};
	paramData getTXGain(int in_chan){return shared_uhd->get_tx_gain(in_chan);};
	paramData getRXRate(int in_chan){return shared_uhd->get_rx_rate(in_chan);};
	paramData getTXRate(int in_chan){return shared_uhd->get_tx_rate(in_chan);};
	bool checkRXChannel(int in_chan){return (in_chan < shared_uhd->get_rx_num_channels());};
	bool checkTXChannel(int in_chan){return (in_chan < shared_uhd->get_tx_num_channels());};
	bool checkRXFreq(paramData in_param){return (in_param.getDouble() == shared_uhd->get_rx_freq_range(in_param.getChannel()).clip(in_param.getDouble()));};
	bool checkTXFreq(paramData in_param){return (in_param.getDouble() == shared_uhd->get_tx_freq_range(in_param.getChannel()).clip(in_param.getDouble()));};
	bool checkRXGain(paramData in_param){return (in_param.getDouble() == shared_uhd->get_rx_gain_range(in_param.getChannel()).clip(in_param.getDouble()));};
	bool checkTXGain(paramData in_param){return (in_param.getDouble() == shared_uhd->get_tx_gain_range(in_param.getChannel()).clip(in_param.getDouble()));};
	bool checkRXRate(paramData in_param){return (in_param.getDouble() == shared_uhd->get_rx_rates(in_param.getChannel()).clip(in_param.getDouble()));};
	bool checkTXRate(paramData in_param){return (in_param.getDouble() == shared_uhd->get_tx_rates(in_param.getChannel()).clip(in_param.getDouble()));};
	void setCustomSDRParameter(std::string name, std::string val, int port);
	std::string getCustomSDRParameter(std::string name);
	void setStreamDataType(streamType in_type){cur_stream_type = in_type;};
};

