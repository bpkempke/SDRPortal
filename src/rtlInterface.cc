/*    SDRPortal - A generic web-based interface for SDRs
 *    Copyright (C) 2013 Ben Kempke (bpkempke@umich.edu)
 *
 *    This program is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "rtlInterface.h"
#include <iostream>

//NOTE: In order to get this to work correctly under a VM, librtlsdr.c requires a longer CONTROL_TIMEOUT value than provided by default

static void *rtlReadProxy(void *in_args){
	rtlInterface *rtl_int = (rtlInterface*)in_args;
	void *return_pointer = rtl_int->rxThread();
	return return_pointer;
}

rtlInterface::rtlInterface(int index) : genericSDRInterface(STREAM_INT8_T){
	rtl_dev = NULL;
	rx_cancel = false;

	//Get information about each RTL device
	int device_count = rtlsdr_get_device_count();
	std::cout << "RTLSDR Device Count: " << device_count << std::endl;

	char vendor[256], product[256], serial[256];
	for(int ii=0; ii < device_count; ii++){
		rtlsdr_get_device_usb_strings(ii, vendor, product, serial);
		std::cout << ii << ": " << vendor << ", " << product << ", SN: " << serial << std::endl;
	}
	std::cout << "Using device " << index << ": " << rtlsdr_get_device_name(index) << std::endl;

	int rtl_ret = rtlsdr_open(&rtl_dev, (uint32_t)index);
	if(rtl_ret != 0){
		if(rtl_ret == -ENOMEM)
			std::cerr << "ERROR: Not enough memory to instantiate rtlsdr" << std::endl;
		else if(rtl_ret == -1)
			std::cerr << "ERROR: rtlsdr device not found" << std::endl;
		exit(1);
	}

	is_receiving = false;
}

rtlInterface::~rtlInterface(){
	rx_cancel = true;
	std::cout << "JOINING" << std::endl;
	pthread_join(rx_listener, NULL);
	std::cout << "CLOSING" << std::endl;
	rtlsdr_close(rtl_dev);
}

void rtlInterface::setRXFreq(paramData in_param){
	std::cout << "setting frequency of " << rtl_dev << " to " << in_param.getUInt32() << std::endl;
	rtlsdr_set_freq_correction(rtl_dev, 50);
	rtlsdr_set_center_freq(rtl_dev, in_param.getUInt32());
}

void rtlInterface::setRXGain(paramData in_param){
	std::cout << "Setting RX Gain" << std::endl;
	//First figure out what the desired gain is
	double desired_gain = in_param.getDouble();

	//Then figure out what gain value it's closest to
	int num_gains = rtlsdr_get_tuner_gains(rtl_dev, NULL);
	int *gains = new int[num_gains];
	rtlsdr_get_tuner_gains(rtl_dev, gains);
	int resulting_gain;
	for(resulting_gain=0; resulting_gain < num_gains; resulting_gain++){
		if((double)(gains[resulting_gain])/10 > desired_gain)
			break;
	}

	//Then set it
	//rtlsdr_set_agc_mode(rtl_dev, 1);
	rtlsdr_set_tuner_gain_mode(rtl_dev, 0);
	rtlsdr_set_tuner_gain(rtl_dev, gains[resulting_gain]);

	delete [] gains;
}

void rtlInterface::setRXRate(paramData in_param){
	std::cout << "Setting RX Rate" << std::endl;
	rtlsdr_set_sample_rate(rtl_dev, in_param.getUInt32());
}

paramData rtlInterface::getRXFreq(rxtxChanInfo in_chan){
	return paramData(rtlsdr_get_center_freq(rtl_dev));
}

paramData rtlInterface::getRXGain(rxtxChanInfo in_chan){
	int ret_gain = rtlsdr_get_tuner_gain(rtl_dev);
	return paramData((double)(ret_gain)/10, in_chan);
}

paramData rtlInterface::getRXRate(rxtxChanInfo in_chan){
	return paramData(rtlsdr_get_sample_rate(rtl_dev));
}

bool rtlInterface::checkRXChannel(int in_chan){
	if(in_chan == 0)
		return true;
	else
		return false;
}

void rtlInterface::openRXChannel(int in_chan){
	if(!is_receiving){
		is_receiving = true;
		pthread_create(&rx_listener, NULL, rtlReadProxy, (void*)this);
	}
}

bool rtlInterface::checkRXFreq(paramData in_param){
	//Check to see if we can actually tune to the selected frequency
	uint32_t rtl_prev_freq = rtlsdr_get_center_freq(rtl_dev);
	int ret = rtlsdr_set_center_freq(rtl_dev, in_param.getUInt32());

	//Set back to the previous frequency
	rtlsdr_set_center_freq(rtl_dev, rtl_prev_freq);
	
	if(ret != 0) return false;
	else return true;
}

bool rtlInterface::checkRXGain(paramData in_param){
	//First figure out what the desired gain is
	double desired_gain = in_param.getDouble();

	//Then figure out what gain value it's closest to
	int num_gains = rtlsdr_get_tuner_gains(rtl_dev, NULL);
	int *gains = new int[num_gains];
	rtlsdr_get_tuner_gains(rtl_dev, gains);

	//Now check to see if the gain is within the gain range of the device
	bool ret = false;
	if(desired_gain >= (double)(gains[0])/10 && desired_gain <= (double)(gains[num_gains-1])/10)
		ret = true;

	delete [] gains;
	return ret;
}

bool rtlInterface::checkRXRate(paramData in_param){
	//No associated method to do this...
	return true;
}

void rtlInterface::setCustomSDRParameter(std::string name, std::string val, int in_chan){
	if(name == "_RTLAGCTUNER"){
		if(val == "ON")
			rtlsdr_set_tuner_gain_mode(rtl_dev, 1);
		else
			rtlsdr_set_tuner_gain_mode(rtl_dev, 0);
	} else if(name == "_RTLAGC"){
		if(val == "ON")
			rtlsdr_set_agc_mode(rtl_dev, 1);
		else
			rtlsdr_set_agc_mode(rtl_dev, 0);
	}
}

void rtlInterface::disconnect(){
	//TODO: Fill this in...
}

void rtlInterface::connect(){
	//TODO: Fill this in...
}

#define RTL_BUFF_LEN 16384
void *rtlInterface::rxThread(){
	unsigned char read_buffer[RTL_BUFF_LEN];
	char read_translate_buffer[RTL_BUFF_LEN];
	int n_read;
	bool first_time = true;
	int r = rtlsdr_reset_buffer(rtl_dev);
	if(r < 0)
		std::cout << "WARNING: Failed to reset buffers" << std::endl;
	std::cout << "Starting sync read from RTL" << std::endl;
	while(1){
		//Read data synchronously from the RTL device
		int r = rtlsdr_read_sync(rtl_dev, read_buffer, RTL_BUFF_LEN, &n_read);
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

		//Now send the data off to where it needs to go...
		distributeRXData(read_translate_buffer, n_read, 0);
	}
	return NULL;
}
