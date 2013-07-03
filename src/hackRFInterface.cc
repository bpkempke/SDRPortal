#include "hackRFInterface.h"
#include <iostream>

int hrfReadProxy(hackrf_transfer *in_transfer){
	hackRFInterface *hrf_int = (hackRFInterface*)in_transfer.rx_ctx;
	int return_val = hrf_int->rxData(in_transfer);
	return return_val;
}

hackRFInterface::hackRFInterface(int index){
	hrf_dev = NULL;
	rx_cancel = false;

	//Open the only hackrf device connected to this system (no support for multiple hackRFs yet...)
	int result = hackrf_init();
	if( result != HACKRF_SUCCESS ) {
		printf("hackrf_init() failed: %s (%d)\n", hackrf_error_name(result), result);
		exit(1);
	}
	result = hackrf_open(&hrf_dev);
	if( result != HACKRF_SUCCESS ) {
		printf("hackrf_open() failed: %s (%d)\n", hackrf_error_name(result), result);
		exit(1);
	}

	is_receiving = false;
}

hackRFInterface::~hackRFInterface(){
	rx_cancel = true;
	std::cout << "JOINING" << std::endl;
	pthread_join(rx_listener, NULL);
	std::cout << "CLOSING" << std::endl;
	rtlsdr_close(hrf_dev);
}

void hackRFInterface::setRXFreq(paramData in_param){
	std::cout << "setting frequency of " << (int)hrf_dev << " to " << in_param.getUInt32() << std::endl;
	rtlsdr_set_freq_correction(hrf_dev, 50);
	rtlsdr_set_center_freq(hrf_dev, in_param.getUInt32());
}

void hackRFInterface::setRXGain(paramData in_param){
	uint32_t in_gain = in_param.getUInt32();

	//TODO: Need to figure out a way to calculate optimal LNA and VGA gains
	unsigned int lna_gain = 8;
	unsigned int vga_gain = (unsigned int)in_gain;

	/* range 0-62 step 2db */
	result = hackrf_set_vga_gain(hrf_dev, vga_gain);
	/* range 0-40 step 8db */
	result |= hackrf_set_lna_gain(hrf_dev, lna_gain);
}

void hackRFInterface::setRXRate(paramData in_param){
	std::cout << "Setting RX Rate" << std::endl;
	uint32_t sample_rate_hz = in_param.getUInt32();

	//First set the sample rate to the desired sample rate
	int result = hackrf_set_sample_rate_manual(hrf_dev, sample_rate_hz, 1);
	if( result != HACKRF_SUCCESS ) {
		printf("hackrf_sample_rate_set() failed: %s (%d)\n", hackrf_error_name(result), result);
		exit(1);
	}

	//Set baseband filter bandwidth based on sample rate given
	uint32_t baseband_filter_bw_hz = hackrf_compute_baseband_filter_bw_round_down_lt(sample_rate_hz);
	result = hackrf_set_baseband_filter_bandwidth(device, baseband_filter_bw_hz);
	if( result != HACKRF_SUCCESS ) {
		printf("hackrf_baseband_filter_bandwidth_set() failed: %s (%d)\n", hackrf_error_name(result), result);
		exit(1);
	}
}

paramData hackRFInterface::getRXFreq(rxtxChanInfo in_chan){
	return paramData(rtlsdr_get_center_freq(hrf_dev));
}

paramData hackRFInterface::getRXGain(rxtxChanInfo in_chan){
	int ret_gain = rtlsdr_get_tuner_gain(hrf_dev);
	return paramData((double)(ret_gain)/10, in_chan);
}

paramData hackRFInterface::getRXRate(rxtxChanInfo in_chan){
	return paramData(rtlsdr_get_sample_rate(hrf_dev));
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
		result |= hackrf_start_rx(hrf_dev, rx_callback, (void*)this);
	}
}

bool hackRFInterface::checkRXFreq(paramData in_param){
	//Check to see if we can actually tune to the selected frequency
	uint32_t rtl_prev_freq = rtlsdr_get_center_freq(hrf_dev);
	int ret = rtlsdr_set_center_freq(hrf_dev, in_param.getUInt32());

	//Set back to the previous frequency
	rtlsdr_set_center_freq(hrf_dev, rtl_prev_freq);
	
	if(ret != 0) return false;
	else return true;
}

bool hackRFInterface::checkRXGain(paramData in_param){
	//First figure out what the desired gain is
	double desired_gain = in_param.getDouble();

	//Then figure out what gain value it's closest to
	int num_gains = rtlsdr_get_tuner_gains(hrf_dev, NULL);
	int *gains = new int[num_gains];
	rtlsdr_get_tuner_gains(hrf_dev, gains);

	//Now check to see if the gain is within the gain range of the device
	bool ret = false;
	if(desired_gain >= (double)(gains[0])/10 && desired_gain <= (double)(gains[num_gains-1])/10)
		ret = true;

	delete [] gains;
	return ret;
}

bool hackRFInterface::checkRXRate(paramData in_param){
	//No associated method to do this...
	return true;
}

void hackRFInterface::setCustomSDRParameter(std::string name, std::string val, int in_chan){
	//TODO: Anything needed here??
}

void *hackRFInterface::rxData(hackrf_transfer *in_transfer){
	static uint8_t *read_translate_buffer;
	static int read_translate_length = 0;
	int n_read;
	bool first_time = true;
	int r = rtlsdr_reset_buffer(hrf_dev);
	if(r < 0)
		std::cout << "WARNING: Failed to reset buffers" << std::endl;
	std::cout << "Starting sync read from RTL" << std::endl;
	while(1){
		//Read data synchronously from the RTL device
		int r = rtlsdr_read_sync(hrf_dev, read_buffer, RTL_BUFF_LEN, &n_read);
		if (r < 0) {
			std::cerr << "WARNING: sync read failed." << std::endl;
			break;
		}
		if(rx_cancel) break;
		if(first_time)
			std::cout << "GOT SOME SAMPLES!" << std::endl;
		first_time = false;

		//Translate to normal signed bytes by subtracting 127 from the data stream
		for(int ii=0; ii < n_read; ii++)
			read_translate_buffer[ii] = (char)(read_buffer[ii])-127;

		//Now figure out what we need to change the data to
		std::vector<primType> resulting_prim_types = getResultingPrimTypes(0);
		for(unsigned int ii=0; ii < resulting_prim_types.size(); ii++){
			int num_translated_bytes = str_converter.convertTo(read_translate_buffer, RTL_BUFF_LEN, resulting_prim_types[ii]);
			distributeRXData(str_converter.getResult(), num_translated_bytes, 0, resulting_prim_types[ii]);
		}
	}
	return NULL;
}
