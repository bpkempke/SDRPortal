#include "socketInterface.h"
#include "generic_sdr_interface.h"
#include <uhd/usrp/multi_usrp.hpp>
#include <map>

//Temporary uhdInterface definition until later on
class uhdInterface;

//Enumeration type for control connection types
enum controlIntType{
	CONTROL_DATA,
	CONTROL_CMD
};

//Abstract base class for interpreting data streams
class connectionInterpreter{
protected:
	uhdInterface *uhd_int;
	fdInterface *upstream_int;
public:
	connectionInterpreter(uhdInterface *in_uhd_int, fdInterface *in_upstream_int){
		uhd_int = in_uhd_int;
		upstream_int = in_upstream_int;
	};
	fdInterface *getUpstreamInt(){return upstream_int;}
	virtual void parse(char *buffer, int num_bytes, fdInterface *from_interface, int secondary_id) =0;
};

class dataConnectionInterpreter : public connectionInterpreter{
public:
	dataConnectionInterpreter(uhdInterface *in_uhd_int, fdInterface *in_upstream_int) : connectionInterpreter(in_uhd_int, in_upstream_int){};
	void parse(char *buffer, int num_bytes, fdInterface *from_interface, int secondary_id);
};

class cmdConnectionInterpreter : public connectionInterpreter{
private:
	int cur_channel;
public:
	cmdConnectionInterpreter(uhdInterface *in_uhd_int, fdInterface *in_upstream_int) : connectionInterpreter(in_uhd_int, in_upstream_int){cur_channel = 0;};
	void parse(char *buffer, int num_bytes, fdInterface *from_interface, int secondary_id);
};

class uhdControlConnection : public fdInterface{
public:
	uhdControlConnection(uhdInterface *in_parent_int, fdInterface *in_int, controlIntType in_int_type);
	controlIntType getIntType();
	void fdBytesReceived(char *buffer, int num_bytes, fdInterface *from_interface, int secondary_id);
	void dataFromUpstream(const char *data, int num_bytes, fdInterface *from_interface);
	void registerDownstreamInterface(fdInterface *in_thread);
private:		
	fdInterface *downstream_int;
	uhdInterface *parent_int;
	int cur_secondary_id;
	controlIntType int_type;
	std::map<int,connectionInterpreter*> interpreters;
};

//This is utilizing the 'Curiously Recurring Template Pattern'
class uhdInterface : public fdInterface, public genericSDRInterface<uhdInterface>{
public:
	//Constructor
	uhdInterface(std::string args, std::string tx_subdev, std::string rx_subdev, std::string tx_ant, std::string rx_ant, double tx_rate, double rx_rate, double tx_freq, double rx_freq, double tx_gain, double rx_gain, bool codec_highspeed);

	//Interfacing methods
	void parseControlStream(std::string in_data);
	
	//Transmit data methods
	void txStart(){};
	void txData(std::complex<float> *tx_data_iq, int num_samples);
	void txEnd();

	void queueTXSamples(std::complex<float> *in_samples, int num_samples);

	//Receive data methods
	void rxStart();
	int rxData(std::complex<int16_t> *rx_data_iq, int num_samples);
	void rxEnd();
	void rxThread();
	void txThread();

	//Accessor method for underlying uhd object
	uhd::usrp::multi_usrp::sptr getUHDObject(){return shared_uhd;};

	//Connection add/removal operations
	void registerDownstreamControlInterface(fdInterface *in_int, controlIntType in_int_type);
	
	//Certain functions inherited from fdInterface class
	void fdBytesReceived(char *buffer, int num_bytes, fdInterface *from_interface, int secondary_id);


private:
	void writeMAX2829Reg(uint32_t value);
	//Pointer to the created uhd object.  TODO: Maybe this should be a superclass to make things more general?
	uhd::usrp::multi_usrp::sptr shared_uhd;

	//Pointers to the tx and rx streams
	uhd::tx_streamer::sptr tx_stream;
	uhd::rx_streamer::sptr rx_stream;
	uhd::tx_metadata_t tx_md;
	uhd::rx_metadata_t rx_md;

	//Queue of samples to transmit
	std::vector<std::complex<float> > tx_queue;

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

