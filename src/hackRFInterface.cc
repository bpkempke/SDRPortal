#include "hackRFInterface.h"
#include <iostream>

int hrfRxProxy(hackrf_transfer *in_transfer){
	hackRFInterface *hrf_int = (hackRFInterface*)in_transfer->rx_ctx;
	return hrf_int->rxData(in_transfer);
}

int hrfTxProxy(hackrf_transfer *in_transfer){
	hackRFInterface *hrf_int = (hackRFInterface*)in_transfer->tx_ctx;
	return hrf_int->txData(in_transfer);
}

hackRFInterface::hackRFInterface(){
	hrf_dev = NULL;
	rx_cancel = false;
	read_translate_buffer = NULL;
	read_translate_length = 0;

	//Open the only hackrf device connected to this system (no support for multiple hackRFs yet...)
	int result = hackrf_init();
	handleHRFResult("hackrf_init", result, true);

	result = hackrf_open(&hrf_dev);
	handleHRFResult("hackrf_open", result, true);

	std::cout << "Opened a connection to a HackRF" << std::endl;

	is_receiving = false;
	is_transmitting = false;
}

hackRFInterface::~hackRFInterface(){
	int result;
	
	//Stop streaming RX/TX
	if(is_receiving){
		result = hackrf_stop_rx(hrf_dev);
		handleHRFResult("hackrf_stop_rx", result, false);
	}
	if(is_transmitting){
		result = hackrf_stop_tx(hrf_dev);
		handleHRFResult("hackrf_stop_tx", result, false);
	}

	//Now perform final close operations
	result = hackrf_close(hrf_dev);
	handleHRFResult("hackrf_close", reuslt, false);

	hackrf_exit();
}

void hackRFInterface::setRXFreq(paramData in_param){
	uint64_t freq_hz = in_param.getUInt64();
	int result = hackrf_set_freq(hrf_dev, freq_hz);
	handleHRFResult("hackrf_set_freq", result, true);

	//Store freq for later retrieval (as there's no getters in the hackrf api)
	cur_freq = freq_hz;
}

void hackRFInterface::setRXGain(paramData in_param){
	uint32_t in_gain = in_param.getUInt32();

	//TODO: Need to figure out a way to calculate optimal LNA and VGA gains
	unsigned int lna_gain = 8;
	unsigned int vga_gain = (unsigned int)in_gain;

	/* range 0-62 step 2db */
	int result = hackrf_set_vga_gain(hrf_dev, vga_gain);
	/* range 0-40 step 8db */
	result |= hackrf_set_lna_gain(hrf_dev, lna_gain);

	//Store gain for later retrieval (as there's no getters in the hackrf api)
	cur_rx_gain = in_gain;
}

void hackRFInterface::setRXRate(paramData in_param){
	std::cout << "Setting RX Rate" << std::endl;
	uint32_t sample_rate_hz = in_param.getUInt32();

	//First set the sample rate to the desired sample rate
	int result = hackrf_set_sample_rate_manual(hrf_dev, sample_rate_hz, 1);
	handleHRFResult("hackrf_set_sample_rate_manual", result, true);

	//Set baseband filter bandwidth based on sample rate given
	uint32_t baseband_filter_bw_hz = hackrf_compute_baseband_filter_bw_round_down_lt(sample_rate_hz);
	result = hackrf_set_baseband_filter_bandwidth(hrf_dev, baseband_filter_bw_hz);
	handleHRFResult("hackrf_set_baseband_filter_bandwidth", result, true);

	//Store gain for later retrieval (as there's no getters in the hackrf api)
	cur_rate = sample_rate_hz;
}

paramData hackRFInterface::getRXFreq(rxtxChanInfo in_chan){
	return paramData(cur_freq);
}

paramData hackRFInterface::getRXGain(rxtxChanInfo in_chan){
	return paramData(cur_rx_gain);
}

paramData hackRFInterface::getRXRate(rxtxChanInfo in_chan){
	return paramData(cur_rate);
}

bool hackRFInterface::checkRXChannel(int in_chan){
	if(in_chan == 0)
		return true;
	else
		return false;
}

void hackRFInterface::openRXChannel(int in_chan){
	if(!is_receiving){
		is_receiving = true;
		int result = hackrf_start_rx(hrf_dev, hrfRxProxy, (void*)this);
		handleHRFResult("hackrf_start_rx", result, true);
	}
}

bool hackRFInterface::checkRXFreq(paramData in_param){
	uint64_t cand_freq = in_param.getUInt64();
	if(cand_freq > 6e9 || cand_freq < 100e6)
		return false;
	else return true;
}

bool hackRFInterface::checkRXGain(paramData in_param){
	//First figure out what the desired gain is
	uint32_t desired_gain = in_param.getUInt32();

	//There are only certain RX gains supported
	if((desired_gain & 1) || desired_gain > 62)
		return false;
	else return true;
}

bool hackRFInterface::checkRXRate(paramData in_param){
	//No associated method to do this...
	return true;
}

void hackRFInterface::setCustomSDRParameter(std::string name, std::string val, int in_chan){
	//TODO: Anything needed here??
	if(name == "_HACKRF_AMP"){
		int result = HACKRF_SUCCESS;
		if(val == "ON")
			result = hackrf_set_amp_enable(hrf_dev, (uint8_t)(1));
		else if(val == "OFF")
			result = hackrf_set_amp_enable(hrf_dev, (uint8_t)(0));
		handleHRFResult("hackrf_set_amp_enable", result, true);
	}
}

int hackRFInterface::rxData(hackrf_transfer *in_transfer){
	int n_read = in_transfer->valid_length;
	static bool first_time = true;

	if(first_time)
		std::cout << "GOT SOME SAMPLES!" << std::endl;
	first_time = false;
	std::cout << "GOT " << n_read << " SAMPLES" << std::endl;

	//Resize translate buffer if it's too small
	if(in_transfer->buffer_length > read_translate_length){
		delete [] read_translate_buffer;
		read_translate_length = in_transfer->buffer_length*2;
		read_translate_buffer = new char[read_translate_length];
	}

	//Translate to normal signed bytes by subtracting 127 from the data stream
	for(int ii=0; ii < n_read; ii++)
		read_translate_buffer[ii] = (char)(in_transfer->buffer[ii])-127;

	//Now figure out what we need to change the data to
	std::vector<primType> resulting_prim_types = getResultingPrimTypes(0);
	for(unsigned int ii=0; ii < resulting_prim_types.size(); ii++){
		int num_translated_bytes = str_converter.convertTo(read_translate_buffer, n_read, resulting_prim_types[ii]);
		distributeRXData(str_converter.getResult(), num_translated_bytes, 0, resulting_prim_types[ii]);
	}
	return 0;
}
	
void hackRFInterface::handleHRFResult(const char *func_name, int result, bool force_kill){
	if(result != HACKRF_SUCCESS){
		printf("%s() failed: %s (%d)\n", func_name, hackrf_error_name((hackrf_error)result), result);
		if(force_kill) exit(1);
	}
}

/***********************
 * hackRF TX functions *
 ***********************/
void hackRFInterface::startTX(){
	//No thread necessary, just need to set up the hackRF to transmit if it isn't already...
	if(!is_transmitting){
		int result = hackrf_start_tx(hrf_dev, hrfTxProxy, (void*)this);
		handleHRFResult("hackrf_start_tx", result, true);
		is_transmitting = true; //TODO: Need to protect these with mutexes
	}
}

void hackRFInterface::stopTX(){
	is_transmitting = false; //TODO: Need to protect these with mutexes
	int result = hackrf_stop_tx(hrf_dev);
	handleHRFResult("hackrf_stop_tx", result, true);
}

int hackRFInterface::txData(hackrf_transfer *in_transfer){
	//Pull data off of the queue supplied by sockets
	if(tx_queue.size() > 1){
		int valid_txbytes = tx_queue.size() !& 1;
		if(valid_txbytes > in_transfer->buffer_size) valid_txbytes = in_transfer->buffer_size;
		for(int ii=0; ii < valid_txbytes; ii++){
			in_transfer->buffer[ii] = (uint8_t)tx_queue.front() + 127;
			tx_queue.pop();
		}
		in_transfer->valid_length = valid_txbytes;
	} else {
		stopTX();
		in_transfer->valid_length = 0;
	}
	return 0;
}

void hackRFInterface::setTXFreq(paramData in_param){
	//For now, setting the TX frequency is the same as setting the RX frequency...
	setRXFreq(in_param);
}

void hackRFInterface::setTXGain(paramData in_param){
	uint32_t txvga_gain = in_param.getUInt32();
	int result = hackrf_set_txvga_gain(hrf_dev, txvga_gain);
	handleHRFResult("hackrf_set_txvga_gain", result, true);
	cur_tx_gain = txvga_gain;
}

void hackRFInterface::setTXRate(paramData in_param){
	//For now, setting the TX rate is the same as setting the RX rate...
	setRXRate(in_param);
}

paramData hackRFInterface::getTXFreq(rxtxChanInfo in_chan){
	return getRXFreq(in_chan);
}

paramData hackRFInterface::getTXGain(rxtxChanInfo in_chan){
	return paramData(cur_tx_gain);
}

paramData hackRFInterface::getTXRate(rxtxChanInfo in_chan){
	return getRXRate(in_chan);
}

bool hackRFInterface::checkTXChannel(int in_chan){
	//For now, any TX channel maps to the one channel present on the hackRF
	return true;
}

void hackRFInterface::openTXChannel(int in_chan){
	//This doesn't really need anything, as TX is started automatically
}

bool hackRFInterface::checkTXFreq(paramData in_param){
	//hackRF supports frequencies from 5 MHz to 6.8 GHz
	uint64_t cand_freq = in_param.getUint64();
	if(cand_freq >= 5e6L && cand_freq <= 6.8e9L)
		return true;
	else
		return false;
}

bool hackRFInterface::checkTXGain(paramData in_param){
	//Only TX VGA gains which are supported are 0-47dB
	double cand_gain = in_param.getDouble();
	if(cand_gain >= 0 && cand_gain <= 47)
		return true;
	else
		return false;
}

bool hackRFInterface::checkTXRate(paramData in_param){
	//Only sample rates less than 20 MHz are supported by the hackRF
	uint32_t cand_rate = in_param.getUInt32();
	if(cand_rate <= 20e6)
		return true;
	else
		return false;
}

void hackRFInterface::txIQData(void *data, int num_messages, int tx_chan, primType in_type){
	static vector<char> inprogress();

	//Start TX if it hasn't started already
	startTX();

	messageType *in_messages = static_cast<messageType*>(data);
	for(int ii=0; ii < num_messages; ii++){
		//First translate the incoming data, holding on to any which are incomplete
		//TODO: All of this translation/messing around should be done in genericSDRInterface instead...
		


		for(int jj=0; jj < in_messages[ii].
	}

	tx_queue.insert(tx_queue.end(),in_samples,in_samples+num_bytes/sizeof(std::complex<float>));
	//TODO: If there is too much data, block here...
}
