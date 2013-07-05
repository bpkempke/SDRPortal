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
	if( result != HACKRF_SUCCESS ) {
		printf("hackrf_init() failed: %s (%d)\n", hackrf_error_name((hackrf_error)result), result);
		exit(1);
	}
	result = hackrf_open(&hrf_dev);
	if( result != HACKRF_SUCCESS ) {
		printf("hackrf_open() failed: %s (%d)\n", hackrf_error_name((hackrf_error)result), result);
		exit(1);
	}

	is_receiving = false;
	is_transmitting = false;
}

hackRFInterface::~hackRFInterface(){
	int result;
	
	//Stop streaming RX/TX
	if(is_receiving){
		result = hackrf_stop_rx(hrf_dev);
		if( result != HACKRF_SUCCESS )
			printf("hackrf_stop_rx() failed: %s (%d)\n", hackrf_error_name((hackrf_error)result), result);
	}
	if(is_transmitting){
		result = hackrf_stop_tx(hrf_dev);
		if( result != HACKRF_SUCCESS )
			printf("hackrf_stop_tx() failed: %s (%d)\n", hackrf_error_name((hackrf_error)result), result);
	}

	//Now perform final close operations
	result = hackrf_close(hrf_dev);
	if( result != HACKRF_SUCCESS ) 
		printf("hackrf_close() failed: %s (%d)\n", hackrf_error_name((hackrf_error)result), result);
	hackrf_exit();
}

void hackRFInterface::setRXFreq(paramData in_param){
	uint32_t freq_hz = in_param.getUInt32();
	int result = hackrf_set_freq(hrf_dev, (uint64_t)freq_hz);
	if( result != HACKRF_SUCCESS ) {
		printf("hackrf_set_freq() failed: %s (%d)\n", hackrf_error_name((hackrf_error)result), result);
		exit(1);
	}

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
	cur_gain = in_gain;
}

void hackRFInterface::setRXRate(paramData in_param){
	std::cout << "Setting RX Rate" << std::endl;
	uint32_t sample_rate_hz = in_param.getUInt32();

	//First set the sample rate to the desired sample rate
	int result = hackrf_set_sample_rate_manual(hrf_dev, sample_rate_hz, 1);
	if( result != HACKRF_SUCCESS ) {
		printf("hackrf_sample_rate_set() failed: %s (%d)\n", hackrf_error_name((hackrf_error)result), result);
		exit(1);
	}

	//Set baseband filter bandwidth based on sample rate given
	uint32_t baseband_filter_bw_hz = hackrf_compute_baseband_filter_bw_round_down_lt(sample_rate_hz);
	result = hackrf_set_baseband_filter_bandwidth(hrf_dev, baseband_filter_bw_hz);
	if( result != HACKRF_SUCCESS ) {
		printf("hackrf_baseband_filter_bandwidth_set() failed: %s (%d)\n", hackrf_error_name((hackrf_error)result), result);
		exit(1);
	}

	//Store gain for later retrieval (as there's no getters in the hackrf api)
	cur_rate = sample_rate_hz;
}

paramData hackRFInterface::getRXFreq(rxtxChanInfo in_chan){
	return paramData(cur_freq);
}

paramData hackRFInterface::getRXGain(rxtxChanInfo in_chan){
	return paramData(cur_gain);
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
		if( result != HACKRF_SUCCESS ) {
			printf("hackrf_start_?x() failed: %s (%d)\n", hackrf_error_name((hackrf_error)result), result);
			exit(1);
		}
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
		if( result != HACKRF_SUCCESS ) {
			printf("hackrf_set_amp_enable() failed: %s (%d)\n", hackrf_error_name((hackrf_error)result), result);
			exit(1); //TODO: Should these throw parameter errors instead?
		}
	}
}

int hackRFInterface::rxData(hackrf_transfer *in_transfer){
	int n_read = in_transfer->valid_length;
	static bool first_time = true;

	if(first_time)
		std::cout << "GOT SOME SAMPLES!" << std::endl;
	first_time = false;

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

int hackRFInterface::txData(hackrf_transfer *in_transfer){
	//TODO: Fill this in...
	return 0;
}
