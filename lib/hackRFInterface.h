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

#ifndef HACKRF_INTERFACE_H
#define HACKRF_INTERFACE_H

#include <queue>
#include "genericSDRInterface.h"
#include "genericSocketInterface.h"
#include "hierarchicalDataflowBlock.h"
#include "streamConverter.h"
#include <libhackrf/hackrf.h>

class hackRFInterface : public genericSDRInterface{
public:
	hackRFInterface();
	virtual ~hackRFInterface();

	int rxData(hackrf_transfer *in_transfer);
	int txData(hackrf_transfer *in_transfer);

	//Certain functions inherited from genericSDRInterface class
	virtual void setRXFreq(paramData in_param);
	virtual void setRXGain(paramData in_param);
	virtual void setRXRate(paramData in_param);
	virtual paramData getRXFreq(rxtxChanInfo in_chan);
	virtual paramData getRXGain(rxtxChanInfo in_chan);
	virtual paramData getRXRate(rxtxChanInfo in_chan);
	virtual bool checkRXChannel(int in_chan);
	virtual void openRXChannel(int in_chan);
	virtual bool checkRXFreq(paramData in_param);
	virtual bool checkRXGain(paramData in_param);
	virtual bool checkRXRate(paramData in_param);

	virtual void setTXFreq(paramData in_param);
	virtual void setTXGain(paramData in_param);
	virtual void setTXRate(paramData in_param);
	virtual paramData getTXFreq(rxtxChanInfo in_chan);
	virtual paramData getTXGain(rxtxChanInfo in_chan);
	virtual paramData getTXRate(rxtxChanInfo in_chan);
	virtual bool checkTXChannel(int in_chan);
	virtual void openTXChannel(int in_chan);
	virtual bool checkTXFreq(paramData in_param);
	virtual bool checkTXGain(paramData in_param);
	virtual bool checkTXRate(paramData in_param);
	virtual void txIQData(void *data, int num_bytes, int tx_chan);

	virtual void setCustomSDRParameter(std::string name, std::string val, int in_chan);
	virtual void disconnect();
	virtual void connect();

private:
	void handleHRFResult(const char *func_name, int result, bool force_kill);
	void stopTX();
	void startTX();

	hackrf_device *hrf_dev;
	bool is_receiving, is_transmitting;
	bool rx_cancel;
	pthread_t rx_listener;
	char *read_translate_buffer;
	int read_translate_length;

	//Variables to store current parameters
	uint32_t cur_rate, cur_rx_gain, cur_tx_gain;
	uint64_t cur_freq;

	std::queue<char> tx_queue;
};

#endif
