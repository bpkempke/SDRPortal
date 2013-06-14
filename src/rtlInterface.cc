#include "rtlInterface.h"
#include <iostream>

static void *rtlReadProxy(void *in_args){
	rtlInterface *rtl_int = (rtlInterface*)in_args;
	void *return_pointer = rtl_int->rxThread();
	return return_pointer;
}

rtlInterface::rtlInterface(int index){
	//TODO: Maybe some error checking with the return values?
	rtlsdr_open(&rtl_dev, (uint32_t)index);

	is_receiving = false;
}

rtlInterface::~rtlInterface(){
	if(is_receiving)
		rtlsdr_cancel_async(rtl_dev);
	rtlsdr_close(rtl_dev);
}

void rtlInterface::setRXFreq(paramData in_param){
	rtlsdr_set_center_freq(rtl_dev, in_param.getUInt32());
}

void rtlInterface::setRXGain(paramData in_param){
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
	rtlsdr_set_tuner_gain_mode(rtl_dev, 1);
	rtlsdr_set_tuner_gain(rtl_dev, gains[resulting_gain]);

	delete gains;
}

void rtlInterface::setRXRate(paramData in_param){
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
	//TODO: Might be able to do better than this depending on which device we're using, etc.
	return true;
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

	delete gains;
	return ret;
}

bool rtlInterface::checkRXRate(paramData in_param){
	//No associated method to do this...
	return true;
}

void rtlInterface::setCustomSDRParameter(std::string name, std::string val, int in_chan){
	//TODO: Things like AGC, etc?
	//int rtlsdr_set_agc_mode(rtlsdr_dev_t *dev, int on);
}

#define RTL_BUFF_LEN 512
void *rtlInterface::rxThread(){
	unsigned char read_buffer[RTL_BUFF_LEN];
	char read_translate_buffer[RTL_BUFF_LEN];
	int n_read;
	while(1){
		//Read data synchronously from the RTL device
		int r = rtlsdr_read_sync(rtl_dev, read_buffer, RTL_BUFF_LEN, &n_read);
		if (r < 0) {
			std::cerr << "WARNING: sync read failed." << std::endl;
			break;
		}

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
