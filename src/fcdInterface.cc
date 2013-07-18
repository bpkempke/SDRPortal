
#include <fcd.h>
#include <fcdhidcmd.h>
#include <portaudio.h>

PaStreamCallback portaudio_source_callback;

int fcdReadProxy(const void *in_buffer, void *out_buffer, unsigned long num_frames, const PaStreamCallbackTimeInfo* time_info, PaStreamCallbackFlags flags, void *in_arg){

	fcdInterface *fcd_int = (fcdInterface*)in_arg;
	return fcd_int->rxData(in_buffer, num_frames);
}

fcdInterface::fcdInterface(const char *device_name) : genericSDRInterface(STREAM_FLOAT_T){
	//Set up portaudio stuff first
	PaError result = Pa_Initialize();
	handleFCDResult("Pa_Initialize", result, true);

	int num_devices = Pa_GetDeviceCount();
	if(num_devices <= 0){
		printf("No valid FCDs available\n");
		exit(1);
	}

	//Check for the device in those which are available
	bool found_device = false;
	int ii;
	PaDeviceInfo *cur_device_info;
	for(ii=0; ii < num_devices; ii++){
		cur_device_info = Pa_GetDeviceInfo(ii);
		if(strstr(cur_device_info->name, device_name)){
			//Found the device, stop searching
			break;
		}
	}
	if(!found_device){
		printf("Couldn't find specified device: %s\n", device_name);
		exit(1);
	} else {
		PaStreamParameters input_params;
		input_params.device = ii;
		input_params.channelCount = 2;
		input_params.sampleFormat = paFloat32;//TODO: Is this the best type?
		input_params.suggestedLatench = cur_device_info->defaultLowInputLatency;
		input_params.hostApiSpecificStreamInfo = NULL;

		//Start streaming now that we have collected all the important info
		result = Pa_OpenStream(fcd_dev, input_params, NULL, 96e3, 2048, paClipOff, &fcdReadProxy, (void*)this);
		handleFCDResult("Pa_OpenStream", result, true);
		result = Pa_StartStream(fcd_dev);
		handleFCDResult("Pa_StartStream", result, true);
	}
}

void fcdInterface::handleFCDResult(const char *func_name, PaError result, bool force_kill){
	if(result < 0){
		printf("%s() failed: %s (%d)\n", func_name, Pa_GetErrorText(result), (int)result);
		if(force_kill) exit(1);
	}
}

fcdInterface::~fcdInterface(){
	Pa_StopStream(fcd_dev);
	Pa_CloseStream(fcd_dev);
	Pa_Terminate();
}

void fcdInterface::setRXFreq(paramData in_param){
	uint32_t target_freq = in_param.getUInt32();
	fcdAppSetFreq((int)target_freq);
	cur_freq = target_freq;
}

void fcdInterface::setRXGain(paramData in_param){
	double target_gain = in_param.getDouble();
	unsigned char g;

	/* convert to nearest discrete value */
	if (gain > 27.5)       g = 14; // 30.0 dB
	else if (gain > 22.5)  g = 13; // 25.0 dB
	else if (gain > 18.75) g = 12; // 20.0 dB
	else if (gain > 16.25) g = 11; // 17.5 dB
	else if (gain > 13.75) g = 10; // 15.0 dB
	else if (gain > 11.25) g = 9;  // 12.5 dB
	else if (gain > 8.75)  g = 8;  // 10.0 dB
	else if (gain > 6.25)  g = 7;  // 7.5 dB
	else if (gain > 3.75)  g = 6;  // 5.0 dB
	else if (gain > 1.25)  g = 5;  // 2.5 dB
	else if (gain > -1.25) g = 4;  // 0.0 dB
	else if (gain > -3.75) g = 1;  // -2.5 dB
	else                   g = 0;  // -5.0 dB
	
	fme = fcdAppSetParam(FCD_CMD_APP_SET_LNA_GAIN, &g, 1);

	//TODO: Figure out how to integrate mixer gain in here as well...
}


void fcdInterface::setRXRate(paramData in_param){
	//The RX rate is always set to 96 kHz... Need to do rate conversion elsewhere if we need it...
}

//TODO: What about the FCD Pro+???

paramData fcdInterface::getRXFreq(rxtxChanInfo in_chan){
}

paramData fcdInterface::getRXGain(rxtxChanInfo in_chan){
}

paramData fcdInterface::getRXRate(rxtxChanInfo in_chan){
}

bool fcdInterface::checkRXChannel(int in_chan){
	if(in_chan == 0)
		return true;
	else
		return false;
}

void fcdInterface::openRXChannel(int in_chan){
	//Should already be receiving...
}

bool fcdInterface::checkRXFreq(paramData in_param){
	uint32_t desired_freq = in_param.getUInt32();

	//FCD only supports:
	//  64-1100 MHz
	//  1270-1700 MHz
	if((desired_freq >= 64e6 && desired_freq <= 1100e6) || (desired_freq >= 1270e6 && desired_freq <= 1700e6))
		return true;
	else
		return false;
}

bool fcdInterface::checkRXGain(paramData in_param){
	//Really everything gets clipped anyways, so anything is fine
	return true;
}

bool fcdInterface::checkRXRate(paramData in_param){
	//No associated method to do this...
	return true;
}

void fcdInterface::setCustomSDRParameter(std::string name, std::string val, int in_chan){
	//TODO: Anything here???
}

#define RTL_BUFF_LEN 16384
void *fcdInterface::rxData(const char *in_buffer, unsigned long num_frames){

	//Each frame consists of one I/Q float pair
	distributeRXData(in_buffer, num_frames*sizeof(float)*2, 0);
	return NULL;
}

