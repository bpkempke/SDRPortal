#include "socketInterface.h"
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
	int cur_rx_channel, cur_tx_channel;
public:
	cmdConnectionInterpreter(uhdInterface *in_uhd_int, fdInterface *in_upstream_int) : connectionInterpreter(in_uhd_int, in_upstream_int){cur_rx_channel = cur_tx_channel = 0;};
	void parse(char *buffer, int num_bytes, fdInterface *from_interface, int secondary_id);
};

class uhdControlConnection : public fdInterface{
public:
	uhdControlConnection(uhdInterface *in_parent_int, fdInterface *in_int, controlIntType in_int_type);
	controlIntType getIntType();
	void fdBytesReceived(char *buffer, int num_bytes, fdInterface *from_interface, int secondary_id);
	void dataFromUpstream(char *data, int num_bytes, fdInterface *from_interface);
	void registerDownstreamInterface(fdInterface *in_thread);
private:		
	fdInterface *downstream_int;
	uhdInterface *parent_int;
	int cur_secondary_id;
	controlIntType int_type;
	std::map<int,connectionInterpreter*> interpreters;
};

class uhdInterface : public fdInterface, public genericSDRInterface{
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
	int rx_chan = 0;
	int tx_chan = 0;

//Virtual memembers inherited from genericSDRInterface
protected:
	void setRXChannel(int chan){rx_chan = chan;};
	void setTXChannel(int chan){tx_chan = chan;};
	void setRXFreq(double freq){shared_uhd->set_rx_freq(rx_chan);};//TODO: Hrmm, this won't work..
	void setTXFreq(double freq){shared_uhd->set_tx_freq(tx_chan);};
	void setRXGain(double gain){shared_uhd->set_rx_gain(gain, rx_chan);};
	void setTXGain(double gain){shared_uhd->set_tx_gain(gain, tx_chan);};
	void setRXRate(double rate){shared_uhd->set_rx_rate(rate, rx_chan);};
	void setTXRate(double rate){shared_uhd->set_tx_rate(rate, tx_chan);};
	int getRXChannel(){return rx_chan;};
	int getTXChannel(){return tx_chan;};
	double getRXFreq(){return shared_uhd->get_rx_freq(rx_chan);};
	double getTXFreq(){return shared_uhd->get_tx_freq(tx_chan);};
	double getRXGain(){return shared_uhd->get_rx_gain(rx_chan);};
	double getTXGain(){return shared_uhd->get_tx_gain(tx_chan);};
	double getRXRate(){return shared_uhd->get_rx_rate(rx_chan);};
	double getTXRate(){return shared_uhd->get_tx_rate(tx_chan);};
	bool checkRXChannel(int chan){return (chan < shared_uhd->get_rx_num_channels());};
	bool checkTXChannel(int chan){return (chan < shared_uhd->get_tx_num_channels());};
	bool checkRXFreq(double freq){return (freq == shared_uhd->get_rx_freq_range(rx_chan).clip(freq));};
	bool checkTXFreq(double freq){return (freq == shared_uhd->get_tx_freq_range(tx_chan).clip(freq));};
	bool checkRXGain(double gain){return (gain == shared_uhd->get_rx_gain_range(rx_chan).clip(gain));};
	bool checkTXGain(double gain){return (gain == shared_uhd->get_tx_gain_range(tx_chan).clip(gain));};
	bool checkRXRate(double rate){return (rate == shared_uhd->get_rx_rates(rx_chan).clip(rate));};
	bool checkTXRate(double rate){return (rate == shared_uhd->get_tx_rates(tx_chan).clip(rate));};
	void setCustomSDRParameter(std::string name, std::string val);
	std::string getCustomSDRParameter(std::string name);
};

